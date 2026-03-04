#include "Start.h"
#include "st7735.h"
#include "keyboard.h"
#include "tjc.h"
#include "msg_type.h"
#include "data_process.h"
#include "Variables.h"
#include "FLASH.h"
#include "LCD.h"
											
void Handle_Start(void)
{
    // 基本变量、标志位、按键引脚初始化
    FlagInit();
    TempInit();
    KeyInit();
    LCD_Init();
    LCD_ShowHome();
		// 设置机器人模式,默认为R1
		LCD_ShowString(10 ,10 ,(uint8_t *)"R1",WHITE ,BLACK ,32 ,0 );
		LCD_ShowString(10 ,42 ,(uint8_t *)"Select Mode...",WHITE ,BLACK ,12 ,0);
		HAL_Delay(100);
		for(;;)
			{
				if(KeyRead(TOP_LEFT)==GPIO_PIN_RESET)	// 切换模式
				{
					flag.RobotMode=!flag.RobotMode;
					if(!flag.RobotMode)
					{
						LCD_ShowString(10 ,10 ,(uint8_t *)"R1",WHITE ,BLACK ,32 ,0 );
						MsgSend("RobotMode=0");	// 同步控制串口屏(小屏显示为主)
						HAL_Delay(20);
					}
					else
					{
						LCD_ShowString(10 ,10 ,(uint8_t *)"R2",WHITE ,BLACK ,32 ,0 );
						MsgSend("RobotMode=1");
						HAL_Delay(20);
					}
					HAL_Delay(200);
				}
				if(KeyRead(TOP_RIGHT)==GPIO_PIN_RESET) // 确定模式
				{
					MsgSend("page main");	// 防止串口屏与小屏不同步
					HAL_Delay(100);
					if(!flag.RobotMode)	
						MsgSend("click m0,0");	
					else
						MsgSend("click m1,0");	
					break;
				}
			}
    JoystickInit();	// 开启摇杆ADC采样
		Usart2DataFrameInit(USART_SUM_CHECK);	// 初始化U2串口数据包框架，包括包头包尾检验位
    LCD_ShowHome();
			
		#if joystickmeasureFlag==1	// 摇杆极限值校准
		JS_measure_show();
    while (joystickmeasureFlag)
    {
        
        joystickLeft1_scan();
        joystickLeft2_scan();
        joystickRight_scan();
    }
    #endif                      
		// 蓝牙初始化，开启U2串口接收
    if (!BW16Init(&bw))
    {
        LCD_ShowString(20, 45, (uint8_t *)"BW16 init error", WHITE, BLACK, 12, 1);
        while (1)
        LCD_ShowChar(80, 58, '!', WHITE, BLACK, 32, 0);
    }
    else
    {
        LCD_ShowString(20, 45, (uint8_t *)"BW16 init success", WHITE, BLACK, 12, 1);
        bw.init = true;
				bw.connect= true;
    }
		
    // 串口屏队列初始化,开启U3串口接收
    if (!TJCInit())
    {
        LCD_ShowString(20, 65, (uint8_t *)"TJC init error", WHITE, BLACK, 12, 1);
        while (1)
        LCD_ShowChar(80, 77, '!', WHITE, BLACK, 32, 0);
    }
    else
        LCD_ShowString(20, 65, (uint8_t *)"TJC init success", WHITE, BLACK, 12, 1);
		
		HAL_Delay(500);
    LCD_ShowHome();      // 清屏
    flag.QUEUErx = true; // 打开队列接收,之前串口接收的数据未入队，用于小屏显示
		flag.QUEUEtx = true; // 打开队列发送
}