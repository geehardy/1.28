#include "keyboard.h"
#include "utility.h"
#include "data_process.h"
#include "adc.h"
#include "tjc.h"
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
    uint8_t ToggleNum = 0x0F;	//0000 1111
    for (Key_E key = TOGGLE_1; key <= TOGGLE_4; key++)
    {
        if (KeyRead(key) == 0)
        { 
            osDelay(10);
            ToggleNum += (0x80 >> (key - TOGGLE_1));
        }
    }
    return ToggleNum; 
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
//    L1.x = ADC_buffer[2];
//    L1.y = ADC_buffer[3];
		#if joystickmeasureFlag==1	// 条件编译
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
		#else 
//        if (flag.JSfast_slow)
//            K = 1600;
//        else
//            K = 800;
        // 将滤波后的L1.x与L1.y映射到-K~K
//        if (flag.JSalonemode)
//        {
//            if (L1.x > L1val[4])
//            {
//                if (abs(K * (L1.x - L1val[4]) / (L1val[1] - L1val[4])) < Dead_zone) // 防止摇杆抖动
//                    JScontrolmsg.velX = 0;
//                else
//                    JScontrolmsg.velX = -K * (L1.x - L1val[4]) / (L1val[1] - L1val[4])-Dead_zone;
//            }
//            else
//            {
//                if (abs(K * (L1val[4] - L1.x) / (L1val[0] - L1val[4])) < Dead_zone) // 防止摇杆抖动
//                    JScontrolmsg.velX = 0;
//                else
//                    JScontrolmsg.velX = -K * (L1val[4] - L1.x) / (L1val[0] - L1val[4])+Dead_zone;
//            }
//        }
//        else
//            JScontrolmsg.velX = 0;
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
				#endif
}
// 左边2号摇杆
void joystickLeft2_scan()
{
    L2.x = LowPassFilter(&left3_LPF, ADC_buffer[4]); // 低通滤波
    L2.y = LowPassFilter(&left4_LPF, ADC_buffer[5]); // 低通滤波
//    L2.x = ADC_buffer[4];
//    L2.y = ADC_buffer[5];
		#if joystickmeasureFlag==1
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
		#else
				if (flag.JSfast_slow)	// 设置快慢模式
            K = 1600;
        else
            K = 800;
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
				
				if(flag.JSalonemode)	//单摇杆模式下，L2 x,y同时处理
				{
					  if (L2.y > L2val[5])
						{
							if (abs(K * (L2.y - L2val[5]) / (L2val[3] - L2val[5])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
							else
                JScontrolmsg.velY = K * (L2.y - L2val[5]) / (L2val[3] - L2val[5])-Dead_zone;
						}
						else
						{
							if (abs(K * (L2val[5] - L2.y) / (L2val[2] - L2val[5])) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
							else
                JScontrolmsg.velY = K * (L2val[5] - L2.y) / (L2val[2] - L2val[5])+Dead_zone;
						}
				}
		#endif
}
// 右边摇杆--作为角速度控制
void joystickRight_scan()
{
    R.x = LowPassFilter(&right1_LPF, ADC_buffer[0]);
    R.y = LowPassFilter(&right2_LPF, ADC_buffer[1]);
//    R.x = ADC_buffer[0];
//    R.y = ADC_buffer[1];
		#if joystickmeasureFlag==1
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
		#else
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
		#endif
}

// 电量检测
uint16_t powervalue;

uint16_t power_scan()
{
    powervalue = (ADC_buffer[6] - MIN_power) * 100 / (MAX_power - MIN_power);
    if (powervalue >= 100)
        return 99;
    return powervalue;
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
				TxtmsgSend("\"抬升\"");
        break;
    } // R1抬升
    case 5:
    {
        Press(TakeCube1);
				TxtmsgSend("\"取方块\"");
        break;
    } // R1取方块
    case 9:
    {
        Press(ArmPre1);
				TxtmsgSend("\"臂准备\"");
        break;
    } // R1臂准备
    case 2:
    {
        Press(ReachOut);
				TxtmsgSend("\"伸出\"");
        break;
    } // 伸出
    case 6:
    {
        Press(PutCube1);
				TxtmsgSend("\"放方块\"");
        break;
    } // R1放方块
    case 10:
    {
        Press(TakeRod);
				TxtmsgSend("\"取杆\"");
        break;
    } // 取杆
    case 3:
    {
        Press(Attack);
				TxtmsgSend("\"攻击\"");
        break;
    } // 攻击
    case 7:
    {
        Press(PassCube);
				TxtmsgSend("\"递方块\"");
        break;
    } // 递方块
    case 11:
    {
        Press(WeaponStand);
				TxtmsgSend("\"武器直立\"");
        break;
    } // 武器直立
    case 4:
    {
        Press(ArmLose);
				TxtmsgSend("\"丢臂\"");
        break;
    } // 丢臂
    case 8:
    {
        Press(ScrReget);
				TxtmsgSend("\"Scr收回\"");
        break;
    } // Scr收回
    case 12:
    {
        Press(Match);
				TxtmsgSend("\"对接\"");
        break;
    } // 对接
    case 13:
    { // 底部1
        Press(Reset1);
				TxtmsgSend("\"复位\"");
        break;
    } // R1复位
    case 14:
    { // 底部2
        Press(ArmReset);
				TxtmsgSend("\"臂复位\"");
        break;
    } // 臂复位
    case 15:
    { // 底部3
        Press(ScrReset);
				TxtmsgSend("\"Scr复位\"");
        break;
    } // Scr复位
    case 16:
    { // 底部4
        Press(Protect);
				TxtmsgSend("\"保护\"");
        break;
    } // 保护
    case 17:
    { // 左上
        Press(ArmEn);
				TxtmsgSend("\"臂使能\"");
        break;
    } // 臂使能
    case 18:
    { // 右上
        Press(ScrEn);
				TxtmsgSend("\"Scr使能\"");	// 为什么keil显示gb2312但是实际编码是utf8
        break;
    } // Scr使能
    case 19:
    { // 左摇杆1
        flag.JSfast_slow = !flag.JSfast_slow;
        osDelay(10);
        Beep(1, 50);
			if(flag.JSfast_slow)
				TxtmsgSend("\"快模式\"");
			else
				TxtmsgSend("\"慢模式\"");
        break;
    } // 快慢模式切换
    case 20:
    { // 左摇杆2
        flag.JSalonemode = !flag.JSalonemode;
				osDelay(10);
        Beep(1, 50);
			if(flag.JSalonemode)
				TxtmsgSend("\"单摇杆\"");
			else
				TxtmsgSend("\"双摇杆\"");
        break;
    } 
    case 21:
    { // 右边摇杆
				flag.LCDfirstshow=true;		
				osDelay(10);
        Beep(1, 50);
				TxtmsgSend("\"刷新屏幕\"");
        break;
    } //刷新tft副屏
    default:
        break;
    }
}
void R1_toggle_function(uint8_t togglenum)
{
    BoolChange(Enable1, (togglenum & 0x8f) >> 7);
		BoolChange(TakeCubeMode, (togglenum & 0x4f) >> 6);
    BoolChange(PassCubeMode, (togglenum & 0x2f) >> 5);
    BoolChange(LiftMode, (togglenum & 0x1f) >> 4);
    osDelay(10);
}
// R2按键功能定义
void R2_key_function(uint8_t keynum)
{
    switch (keynum)
    {
    case 1:
    {
        Press(Cell);
				TxtmsgSend("\"Cell\"");
        break;
    } // Cell
    case 5:
    {
        Press(TakeCube2);
				TxtmsgSend("\"取方块\"");
        break;
    } // R2取方块
    case 9:
    {
        Press(TakeHead); 
				TxtmsgSend("\"取武器头\"");
        break;
    } // 取武器头
    case 2:
    {
//        Press(LiftDown);
				TxtmsgSend("\"空\"");
        break;
    } 
    case 6:
    {
        Press(CarryCube);
				TxtmsgSend("\"载方块\"");
        break;
    } // 载方块
    case 10:
    {
        Press(FreeHead); 
				TxtmsgSend("\"武器头释放\"");
        break;
    }	// 武器头释放
    case 3:
    {
//        Press(TakeHead);
				TxtmsgSend("\"空\"");
        break;
    } 
    case 7:
    {
        Press(PutCube2);
				TxtmsgSend("\"放方块\"");
        break;
    } // R2放方块
    case 11:
    {
        Press(LiftUp2);
				TxtmsgSend("\"抬升\"");
        break;
    }	// R2抬升
    case 4:
    {
//        Press(FreeHead);
				TxtmsgSend("\"空\"");
        break;
    }
    case 8:
    {
        Press(ReadCube);
				TxtmsgSend("\"读取方块\"");
        break;
    }	// 读取方块
    case 12:
    {
        Press(LiftDown);
				TxtmsgSend("\"下降\"");
        break;
    }	// 下降
    case 13:
    {
        Press(Reset2);
				TxtmsgSend("\"复位\"");
        break;
    } // R2复位
    case 14:
    {
        Press(ClearPath);
				TxtmsgSend("\"清空路径\"");
        break;
		}	// 清空路径
    case 15:
    {
        Press(SendPath);
				TxtmsgSend("\"发送路径\"");
        break;
    }// 发送路径
    case 16:
    {
        Press(ActiveSet);
				TxtmsgSend("\"主动置位\"");
        break;
		}	// 主动置位
    case 17:
    {
//        Press();
//				Beep(1, 50);
        break;
    }
    case 18:
    {
        Press(NextStep);
				Beep(1, 50);
				TxtmsgSend("\"下一步\"");
        break;
    } // 下一步信号
    case 19:
    { // 左摇杆1
        flag.JSfast_slow = !flag.JSfast_slow;
        osDelay(10);
        Beep(1, 50);
			if(flag.JSfast_slow)
				TxtmsgSend("\"快模式\"");
			else
				TxtmsgSend("\"慢模式\"");
        break;
    } // 快慢模式切换
    case 20:
    { // 左摇杆2
        flag.JSalonemode = !flag.JSalonemode;
        osDelay(10);
        Beep(1, 50);
			if(flag.JSalonemode)
				TxtmsgSend("\"单摇杆\"");
			else
				TxtmsgSend("\"双摇杆\"");
        break;
    } // 单摇杆模式
    case 21:
    { // 右边摇杆
				flag.LCDfirstshow=true;
				osDelay(10);
        Beep(1, 50);
				TxtmsgSend("\"刷新屏幕\"");
        break;
    } 
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
