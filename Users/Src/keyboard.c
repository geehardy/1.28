#include "keyboard.h"
#include "utility.h"
#include "data_process.h"
#include "adc.h"
#include "cmsis_os2.h"
#include "Variables.h"
#include "bw.h"
#include "msg_type.h"
#include "LCD.h"
#include "math.h"
#include <stdlib.h>
/***********************************key GPIO User Define Begin********************************/
__IO Key_S Key[20];

void KeyInit()
{
    Key[K_C1] = (Key_S){GPIOB, GPIO_PIN_6};
    Key[K_C2] = (Key_S){GPIOB, GPIO_PIN_7};
    Key[K_C3] = (Key_S){GPIOB, GPIO_PIN_8};
    Key[K_C4] = (Key_S){GPIOB, GPIO_PIN_9};
    Key[K_R1] = (Key_S){GPIOB, GPIO_PIN_3};
    Key[K_R2] = (Key_S){GPIOB, GPIO_PIN_4};
    Key[K_R3] = (Key_S){GPIOB, GPIO_PIN_5};
    Key[bottom_1] = (Key_S){GPIOC, GPIO_PIN_6};
    Key[bottom_2] = (Key_S){GPIOC, GPIO_PIN_7};
    Key[bottom_3] = (Key_S){GPIOA, GPIO_PIN_11};
    Key[bottom_4] = (Key_S){GPIOA, GPIO_PIN_12};
    Key[TOP_LEFT] = (Key_S){GPIOC, GPIO_PIN_14};
    Key[TOP_RIGHT] = (Key_S){GPIOC, GPIO_PIN_13};
    Key[JOYSTICK_LEFT1] = (Key_S){GPIOB, GPIO_PIN_10};
    Key[JOYSTICK_LEFT2] = (Key_S){GPIOB, GPIO_PIN_11};
    Key[JOYSTICK_RIGHT] = (Key_S){GPIOA, GPIO_PIN_15};
    Key[TOGGLE_1] = (Key_S){GPIOC, GPIO_PIN_8};
    Key[TOGGLE_2] = (Key_S){GPIOA, GPIO_PIN_8};
    Key[TOGGLE_3] = (Key_S){GPIOA, GPIO_PIN_9};
    Key[TOGGLE_4] = (Key_S){GPIOA, GPIO_PIN_10};
}
/***********************************key GPIO User Define end********************************/

// Read key state
GPIO_PinState KeyRead(Key_E key)
{
    return HAL_GPIO_ReadPin(Key[key].GPIOx, Key[key].GPIO_Pin);
}
void KRSet(Key_E key)
{
    HAL_GPIO_WritePin(Key[key].GPIOx, Key[key].GPIO_Pin, GPIO_PIN_SET);
}
void KRReset(Key_E key)
{
    HAL_GPIO_WritePin(Key[key].GPIOx, Key[key].GPIO_Pin, GPIO_PIN_RESET);
}
/*
bottom_1---13
bottom_2---14
bottom_3---15
bottom_4---16
TOP_LEFT---17
TOP_RIGHT---18
JOYSTICK_LEFT1---19
JOYSTICK_LEFT2---20
JOYSTICK_RIGHT---21
TOGGLE_1---1000 1111
TOGGLE_2---0100 1111
TOGGLE_3---0010 1111
TOGGLE_4---0001 1111
  1  2  3  4
  5  6  7  8
  9  10 11 12
*/
uint8_t _tic = 0;
uint8_t keyturn;
uint8_t KeyScan() 	//矩阵扫描
{
    for (__IO Key_E keyRow = K_R1; keyRow <= K_R3+1; keyRow++)
    {
        keyturn=0;
				KRReset(keyRow);
        for (__IO Key_E key = K_C1; key <= K_C4; key++)
        {
            if (KeyRead(key) == 0)
            {
                while (KeyRead(key) == 0 && _tic++ < 3)
									osDelay(5);
						
                _tic = 0;
                keyturn=(uint8_t)((key - K_C1 + 1) + 4 * (keyRow - K_R1));
								break;
            }
        }
        KRSet(keyRow);
				if(keyturn!=0)
				{
					return keyturn;
				}
    }

    for (Key_E key = bottom_1; key <= JOYSTICK_RIGHT; key++)
    {
        if (KeyRead(key) == 0)
        {
            while (KeyRead(key) == 0 && _tic++ < 3)
                osDelay(5);
            _tic = 0;
            return (uint8_t)(key + 6);
        }
    }
		return 0;
}
uint8_t ToggleScan()
{
    uint8_t ToggleNum1 = 0x0F;	//0000 1111
    for (Key_E key = TOGGLE_1; key <= TOGGLE_4; key++)
    {
        osDelay(10);
        if (KeyRead(key) == 0)
        { 
            osDelay(10);
            ToggleNum1 += (0x80 >> (key - TOGGLE_1));
        }
    }
    return ToggleNum1; 
}
/*霍尔遥感扫描*/
uint16_t ADC_buffer[7];
// LPF left1_LPF;
// LPF left2_LPF;
// LPF left3_LPF;
// LPF left4_LPF;
// LPF right1_LPF;
// LPF right2_LPF;
uint8_t JSdataInitnum = 0;
JS L1;
JS L2;
JS R;
/*
0--右边x
1--右边y
2--左边1号x
3--左边1号y
4--左边2号x
5--左边2号y
6--电量
*/

bool JoystickInit()
{  
		InitLowPassFilter(&left1_LPF, 0.02f, 1.f); // 初始化低通滤波器
		InitLowPassFilter(&left2_LPF, 0.02f, 1.f);
    InitLowPassFilter(&left3_LPF, 0.02f, 1.f);
    InitLowPassFilter(&left4_LPF, 0.02f, 1.f);
    InitLowPassFilter(&right1_LPF, 0.02f, 1.f);
    InitLowPassFilter(&right2_LPF, 0.02f, 1.f);

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_buffer, 7) == HAL_ERROR)    // 开启ADC的DMA传输
        return false;
    else
    {
        while (JSdataInitnum++ < 20) // 让低通滤波器趋于稳定
        {
        L1.x = LowPassFilter(&left1_LPF, ADC_buffer[2]);
        L1.y = LowPassFilter(&left2_LPF, ADC_buffer[3]);
        L2.x = LowPassFilter(&left3_LPF, ADC_buffer[4]);
        L2.y = LowPassFilter(&left4_LPF, ADC_buffer[5]);
        R.x = LowPassFilter(&right1_LPF, ADC_buffer[0]);
        R.y = LowPassFilter(&right2_LPF, ADC_buffer[1]);
    }
        return true;
    }
}
// 存储摇杆极限值
// xmin xmax ymin ymax xmid ymid
//int16_t L1val[6] = {460, 3680, 610, 3670, 2050, 1990};
//int16_t L2val[6] = {420, 3480, 500, 3560, 1950, 2050};
//int16_t Rval[6] = {500, 3620, 410, 3360, 2070, 2100};
int16_t L1val[6] = {0, 4095, 6, 4095, 2000, 2000};
int16_t L2val[6] = {9, 4095, 4, 4095, 2100, 2100};
int16_t Rval[6] = {0, 4095, 0, 4095, 2000, 1900};

// 静止采样数
uint8_t L1_index = 0;
uint8_t L2_index = 0;
uint8_t R_index = 0;
JScontrol_S JScontrolmsg;
uint8_t Dead_zone = 100; // 摇杆死区
short K = 1600;
// 左边一号摇杆
void joystickLeft1_scan()
{
    L1.x = LowPassFilter(&left1_LPF, ADC_buffer[2]); // 低通滤波
    L1.y = LowPassFilter(&left2_LPF, ADC_buffer[3]); // 低通滤波
    L1.x = ADC_buffer[2];
    L1.y = ADC_buffer[3];
    if (joystickmeasureFlag)
    {
        if (L1_index == 0)
        { // 第一次先清零
            L1_index++;
            L1val[0] = 2048;
            L1val[1] = 2048;
            L1val[2] = 2048;
            L1val[3] = 2048;
            L1val[4] = 0;
            L1val[5] = 0;
        }
        else if (L1_index < 101)
						{ // 静置采样100个点取均值作为手柄中心位置
            L1_index++;
            L1val[4] += (L1.x / 100);
            L1val[5] += (L1.y / 100);
						}
						else
						{ // 将手柄推到底作圆周运动，读取极限值
								L1val[0] = (L1.x < L1val[0]) ? L1.x : L1val[0];
								L1val[2] = (L1.y < L1val[2]) ? L1.y : L1val[2];
								L1val[1] = (L1.x > L1val[1]) ? L1.x : L1val[1];
								L1val[3] = (L1.y > L1val[3]) ? L1.y : L1val[3];
						}
    }
    else
    {

        if (flag.JSfast_slow)
            K = 1600;
        else
            K = 800;
        // 将滤波后的L1.x与L1.y映射到-K~K
        if (flag.JSalonemode)
        {
            if (L1.x > L1val[4])
            {
                if (abs(K * (L1.x - L1val[4]) / (L1val[1] - L1val[4])) < Dead_zone) // 防止摇杆抖动
                    JScontrolmsg.velX = 0;
                else
                    JScontrolmsg.velX = -K * (L1.x - L1val[4]) / (L1val[1] - L1val[4])-Dead_zone;
            }
            else
            {
                if (abs(K * (L1val[4] - L1.x) / (L1val[0] - L1val[4])) < Dead_zone) // 防止摇杆抖动
                    JScontrolmsg.velX = 0;
                else
                    JScontrolmsg.velX = -K * (L1val[4] - L1.x) / (L1val[0] - L1val[4])+Dead_zone;
            }
        }
        else
            JScontrolmsg.velX = 0;

        if (L1.y > L1val[5])
        {
            if (abs(K * (L1.y - L1val[5]) / (L1val[3] - L1val[5])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
            else
                JScontrolmsg.velY = K * (L1.y - L1val[5]) / (L1val[3] - L1val[5])-Dead_zone;
        }
        else
        {
            if (abs(K * (L1val[5] - L1.y) / (L1val[2] - L1val[5])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
            else
                JScontrolmsg.velY = K * (L1val[5] - L1.y) / (L1val[2] - L1val[5])+Dead_zone;
        }
    }
}
// 左边2号摇杆
void joystickLeft2_scan()
{
    L2.x = LowPassFilter(&left3_LPF, ADC_buffer[4]); // 低通滤波
    L2.y = LowPassFilter(&left4_LPF, ADC_buffer[5]); // 低通滤波
    L2.x = ADC_buffer[4];
    L2.y = ADC_buffer[5];
    if (joystickmeasureFlag)
    {
        if (L2_index == 0)
        { // 第一次先清零
            L2_index++;
            L2val[0] = 2048;
            L2val[1] = 2048;
            L2val[2] = 2048;
            L2val[3] = 2048;
            L2val[4] = 0;
            L2val[5] = 0;
        }
        else if (L2_index < 101)
        { // 静置采样100个点取均值作为手柄中心位置
            L2_index++;
            L2val[4] += (L2.x / 100);
            L2val[5] += (L2.y / 100);
        }
        else
        { // 将手柄推到底作圆周运动，读取极限值
            L2val[0] = (L2.x < L2val[0]) ? L2.x : L2val[0];
            L2val[2] = (L2.y < L2val[2]) ? L2.y : L2val[2];
            L2val[1] = (L2.x > L2val[1]) ? L2.x : L2val[1];
            L2val[3] = (L2.y > L2val[3]) ? L2.y : L2val[3];
        }
    }
    else
    {
				if (L2.x > L2val[4])
        {
            if (abs(K * (L2.x - L2val[4]) / (L2val[1] - L2val[4])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velX = 0;
            else
                JScontrolmsg.velX = -K * (L2.x - L2val[4]) / (L2val[1] - L2val[4])-Dead_zone;
        }
        else
        {
            if (abs(K * (L2val[4] - L2.x) / (L2val[0] - L2val[4])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velX = 0;
            else
                JScontrolmsg.velX = -K * (L2val[4] - L2.x) / (L2val[0] - L2val[4])+Dead_zone;
        }
    }
}
// 右边摇杆--作为角速度控制
void joystickRight_scan()
{
    R.x = LowPassFilter(&right1_LPF, ADC_buffer[0]);
    R.y = LowPassFilter(&right2_LPF, ADC_buffer[1]);
    R.x = ADC_buffer[0];
    R.y = ADC_buffer[1];
    if (joystickmeasureFlag)
    {
        if (R_index == 0)
        { // 第一次先清零
            R_index++;
            Rval[0] = 2048;
            Rval[1] = 2048;
            Rval[2] = 2048;
            Rval[3] = 2048;
            Rval[4] = 0;
            Rval[5] = 0;
        }
        else if (R_index < 101)
        { // 静置采样100个点取均值作为手柄中心位置
            R_index++;
            Rval[4] += (R.x / 100);
            Rval[5] += (R.y / 100);
        }
        else
        { // 将手柄推到底作圆周运动，读取极限值
            Rval[0] = (R.x < Rval[0]) ? R.x : Rval[0];
            Rval[2] = (R.y < Rval[2]) ? R.y : Rval[2];
            Rval[1] = (R.x > Rval[1]) ? R.x : Rval[1];
            Rval[3] = (R.y > Rval[3]) ? R.y : Rval[3];
        }
    }
    else
    {
        if (R.x > Rval[4])
        {
            if (abs((short)1600 * (R.x - Rval[4]) / (Rval[1] - Rval[4])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.angW = 0;
            else
                JScontrolmsg.angW = -(short)1600 * (R.x - Rval[4]) / (Rval[1] - Rval[4])+Dead_zone;
        }
        else
        {
            if (abs((short)1600 * (Rval[4] - R.x) / (Rval[0] - Rval[4])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.angW = 0;
            else
                JScontrolmsg.angW = -(short)1600 * (Rval[4] - R.x) / (Rval[0] - Rval[4])-Dead_zone;
        }
    }
}

// 电量检测
uint8_t power_scan()
{
    uint8_t power = (ADC_buffer[6] - MIN_power) * 100 / (MAX_power - MIN_power);
    if (power >= 100)
        return 99;
    return power;
}

/***********************************key Function User Define Begin********************************/

// R1按键功能定义
void R1_key_function(uint8_t keynum)
{

    switch (keynum)
    {
    case 1:
    {
        Press(LiftUp1);
        break;
    } // R1抬升
    case 5:
    {
        Press(ArmPre1);
        break;
    } // R1臂准备
    case 9:
    {
        Press(TakeCube1);
        break;
    } // R1取方块
    case 2:
    {
        Press(ReachOut);
        break;
    } // 伸出
    case 6:
    {
        Press(TakeRod);
        break;
    } // 取杆
    case 10:
    {
        Press(PutCube1);
        break;
    } // R1放方块
    case 3:
    {
        Press(Attack);
        break;
    } // 攻击
    case 7:
    {
        Press(Match);
        break;
    } // 对接
    case 11:
    {
        Press(PassCube);
        break;
    } // 递方块
    case 4:
    {
        Press(ArmLose);
        break;
    } // 丢臂
    case 8:
    {
        Press(WeaponStand);
        break;
    } // 武器直立
    case 12:
    {
        Press(ScrReget);
        break;
    } // Scr收回
    case 13:
    { // 底部1
        Press(Reset1);
        break;
    } // R1复位
    case 14:
    { // 底部2
        Press(ArmReset);
        break;
    } // 臂复位
    case 15:
    { // 底部3
        Press(ScrReset);
        break;
    } // Scr复位
    case 16:
    { // 底部4
        Press(Zero);
        break;
    } // 回零
    case 17:
    { // 左上
        Press(ReGPS);
        break;
    } // 重定位
    case 18:
    { // 右上
        Press(SetZero);
        break;
    } // 置零
    case 19:
    { // 左摇杆1
        flag.JSfast_slow = !flag.JSfast_slow;
        Beep(1, 50);
				osDelay(10);
        break;
    } // 快慢模式切换
    case 20:
    { // 左摇杆2
        flag.JSalonemode = !flag.JSalonemode;
        Beep(1, 50);
				osDelay(10);
        break;
    } // 单摇杆模式
    case 21:
    { // 右边摇杆
				flag.LCDfirstshow=true;		
				osDelay(10);
        Beep(1, 50);
        break;
    } //刷新tft副屏
    default:
        break;
    }
}
void R1_toggle_function(uint8_t togglenum)
{
    // 检测4个拨杆的状态 设定工作模式
    BoolChange(Enable1, (togglenum & 0x8f) >> 7);		 // 第1个拨杆 使能信号
		BoolChange(ArmEn, (togglenum & 0x4f) >> 6);  		 // 第2个拨杆 机械臂使能
    BoolChange(ScrEn, (togglenum & 0x2f) >> 5);  		 // 第3个拨杆 Scr使能
    BoolChange(Protect, (togglenum & 0x1f) >> 4);    // 第4个拨杆 保护信号
    osDelay(10);
}
// R2按键功能定义
void R2_key_function(uint8_t keynum)
{
    switch (keynum)
    {
    case 1:
    {
        Press(LiftUp2);
        break;
    } // R2抬升
    case 5:
    {
        Press(TakeCube2);
        break;
    } // R2取方块
    case 9:
    {
        Press(Cell); 
        break;
    } // Cell信号
    case 2:
    {
        Press(LiftDown);
        break;
    } // 下降
    case 6:
    {
        Press(CarryCube);
        break;
    } // 载方块
    case 10:
    {
//        Press(); 
        break;
    }
    case 3:
    {
        Press(TakeHead);
        break;
    } // 取武器头
    case 7:
    {
        Press(PutCube2);
        break;
    } // R2放方块信号
    case 11:
    {
        //Press();
        break;
    }
    case 4:
    {
        Press(FreeHead);
        break;
    }//武器头释放
    case 8:
    {
        Press(ReadCube);
        break;
    }//读取方块信号
    case 12:
    {
        //Press();
        break;
    }
    case 13:
    {
        Press(Reset2);
        break;
    } //R2复位
    case 14:
    {
        Press(ClearPath);
        break;
		}	//清空路径
    case 15:
    {
        Press(SendPath);
        break;
    }//发送路径
    case 16:
    {
        Press(ActiveSet);
        break;
		}	//主动置位
    case 17:
    {
//        Press();
				Beep(1, 50);
        break;
    }
    case 18:
    {
        Press(NextStep);
				Beep(1, 50);
        break;
    } // 下一步信号
    case 19:
    { // 左摇杆1
        flag.JSfast_slow = !flag.JSfast_slow;
        Beep(1, 50);
				osDelay(10);
        break;
    } // 快慢模式切换
    case 20:
    { // 左摇杆2
        flag.JSalonemode = !flag.JSalonemode;
        Beep(1, 50);
				osDelay(10);
        break;
    } // 单摇杆模式
    case 21:
    { // 右边摇杆
				flag.LCDfirstshow=true;
				Beep(1, 50);
				osDelay(10);
        break;
    } // 重新连接
    default:
        break;
    }
}
void R2_toggle_function(uint8_t togglenum)
{
    // 检测4个拨杆的状态 设定工作模式
    BoolChange(WheelEn2, (togglenum & 0x8f) >> 7);    
    BoolChange(InstEn, (togglenum & 0x4f) >> 6); 		
    BoolChange(StairHL, (togglenum & 0x2f) >> 5);            
    BoolChange(TakeCubeHL, (togglenum & 0x2f) >> 5);    	   
    osDelay(10);
}

/***********************************key Function User Define End********************************/
