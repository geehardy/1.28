#include "LCD.h"
#include "msg_type.h"
#include "Variables.h"
#include "bw.h"
#include "cmsis_os.h"
#include "keyboard.h"
#include "Start.h"
void LCD_ShowHome()
{
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
}
void JS_mode_show()
{
    if (flag.JSalonemode != flag.JSalonemode_last || flag.LCDfirstshow)
    {
        if (flag.JSalonemode == true)		//第一次的基本元素
        {
						Draw_Circle(25, 40, 25, WHITE);
            LCD_DrawRectangle(17, 15, 33, 65, BLACK);	 //横屏下 竖的矩形(x1,y1,x2,y2)
            LCD_DrawRectangle(0, 32, 50, 48, BLACK);	// 横屏下 横的矩形
        }
        else
        {
            Draw_Circle(25, 40, 25, BLACK);
            LCD_DrawRectangle(17, 15, 33, 65, WHITE);
            LCD_DrawRectangle(0, 32, 50, 48, WHITE);
					
						LCD_DrawLine(17, 32, 33, 32, BLACK);	//描黑两个矩形交叉的四条线
						LCD_DrawLine(17, 48, 33, 48, BLACK);
						LCD_DrawLine(17, 32, 17, 48, BLACK);
						LCD_DrawLine(33, 32, 33, 48, BLACK);
        }
        flag.JSalonemode_last = flag.JSalonemode;
    }
    if (flag.JSfast_slow != flag.JSfast_slow_last || flag.LCDfirstshow)
    {
        if (flag.JSfast_slow == 1)		//第一次时为快速
            Draw_Circle(25, 40, 15, BLACK);
        else
            Draw_Circle(25, 40, 15, BRRED);
        flag.JSfast_slow_last = flag.JSfast_slow;
    }
}
void R1_Interface()
{
    LCD_ShowHome();
    JS_mode_show();
    LCD_DrawRectangle(90, 84, 150, 96, WHITE);
		LCD_ShowString(5, 5, (uint8_t *)"R1",WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 10, (uint8_t *)"disX:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 30, (uint8_t *)"disY:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 50, (uint8_t *)"Yaw:", WHITE, BLACK, 12, 0);
		LCD_ShowString(55, 70, (uint8_t *)"Pitch:", WHITE, BLACK, 12, 0);
    LCD_ShowString(10, 75, (uint8_t *)"BT", WHITE, BLACK, 12, 0);
    LCD_ShowString(25, 75, (uint8_t *)"RX", WHITE, BLACK, 12, 0);
    LCD_ShowString(40, 75, (uint8_t *)"TX", WHITE, BLACK, 12, 0);
}
void R2_Interface()
{
    LCD_ShowHome();
    JS_mode_show();
    LCD_DrawRectangle(90, 84, 150, 96, WHITE);
		LCD_ShowString(5, 5, (uint8_t *)"R2",WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 10, (uint8_t *)"Pos_X:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 30, (uint8_t *)"Pos_Y:", WHITE, BLACK, 12, 0);
    LCD_ShowString(55, 50, (uint8_t *)"Pos_A:", WHITE, BLACK, 12, 0);
    LCD_ShowString(10, 75, (uint8_t *)"BT", WHITE, BLACK, 12, 0);
    LCD_ShowString(25, 75, (uint8_t *)"RX", WHITE, BLACK, 12, 0);
    LCD_ShowString(40, 75, (uint8_t *)"TX", WHITE, BLACK, 12, 0);
}
void Toggle_status_show()
{
		LCD_Fill(15, 103, 150, 130, BLACK);	// 先将显示拨动开关状态的区域涂黑
		LCD_ShowIntNum(15, 103, (temp.ToggleNum & 0x80) >> 7, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(55, 103, (temp.ToggleNum & 0x40) >> 6, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(93, 103, (temp.ToggleNum & 0x20) >> 5, 1, WHITE, BLACK, 24);
    LCD_ShowIntNum(133, 103, (temp.ToggleNum & 0x10) >> 4, 1, WHITE, BLACK, 24);
}

void BTstatus_show()
{
    if (bw.connect)
        Draw_Circle(15, 90, 3, BLUE);
		else
        Draw_Circle(15, 90, 3, RED);
    if (flag.QUEUErx)
        Draw_Circle(30, 90, 3, YELLOW);
    if (flag.QUEUEtx)
        Draw_Circle(45, 90, 3, GREEN);
}

void LCD_flash()
{
    JS_mode_show();
    temp.L1xNow = 25 + 20 * JScontrolmsg.velX / 1600;
    temp.L1yNow = 40 - 20 * JScontrolmsg.velY / 1600;
    temp.RxNow = 120 + 20 * JScontrolmsg.angW / 1600;
		osDelay(10);
    if (flag.RobotMode == 0)
    {
        LCD_ShowFloatNum1(91, 10, r1backmsg.disX, 7, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 30, r1backmsg.disY, 7, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 50, r1backmsg.Yaw, 7, WHITE, BLACK, 12);
				LCD_ShowFloatNum1(91, 70, r1backmsg.pitch, 7, WHITE, BLACK, 12);
		}
    else
    {
        LCD_ShowIntNum(91, 10, r2backmsg.X, 4, WHITE, BLACK, 12);
        LCD_ShowIntNum(91, 30, r2backmsg.Y, 4, WHITE, BLACK, 12);
        LCD_ShowFloatNum1(91, 50, r2backmsg.angle, 7, WHITE, BLACK, 12);
    }
    Draw_Circle(temp.L1xlast, temp.L1ylast, 1, BLACK);
    Draw_Circle(temp.Rxlast, 90, 3, BLACK);

    Draw_Circle(temp.L1xNow, temp.L1yNow, 1, LIGHTGREEN);
    Draw_Circle(temp.RxNow, 90, 3, LIGHTGREEN);
    temp.L1xlast = temp.L1xNow;
    temp.L1ylast = temp.L1yNow;
    temp.Rxlast = temp.RxNow;
		osDelay(10);
}
void JS_measure_show()
{
    LCD_ShowHome();
    LCD_ShowString(20, 50, (uint8_t *)"joystick measure...", WHITE, BLACK, 12, 0);
}

void Beep(uint8_t times, uint16_t delaytime)
{
    for (uint8_t i = 0; i < times; i++)
    {
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_SET);
        osDelay(delaytime);
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_RESET);
        osDelay(delaytime);
    }
}
void HalBeep(uint8_t times, uint16_t delaytime)
{
    for (uint8_t i = 0; i < times; i++)
    {
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_SET);
        HAL_Delay(delaytime);
        HAL_GPIO_WritePin(BEEPGPIOx, BEEPGPIO_Pin, GPIO_PIN_RESET);
        HAL_Delay(delaytime);
    }
}