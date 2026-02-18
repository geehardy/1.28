/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "msg_type.h"
#include "keyboard.h"
#include "data_process.h"
#include "msg_queue.h"
#include "bw.h"
#include "tjc.h"
#include "string.h"
#include "Variables.h"
#include "LCD.h"
#include "WS2812b.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static uint8_t TJCtxbuffer[128];
static uint8_t BWtxbuffer[128];
//uint32_t last_refresh_time = 0;

char BWrxmsg;
char TJCrxmsg;
/* USER CODE END Variables */
/* Definitions for TJCshow_Task */
osThreadId_t TJCshow_TaskHandle;
const osThreadAttr_t TJCshow_Task_attributes = {
  .name = "TJCshow_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for Key_Task */
osThreadId_t Key_TaskHandle;
const osThreadAttr_t Key_Task_attributes = {
  .name = "Key_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for Joystick_Task */
osThreadId_t Joystick_TaskHandle;
const osThreadAttr_t Joystick_Task_attributes = {
  .name = "Joystick_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for TJCmsg_Task */
osThreadId_t TJCmsg_TaskHandle;
const osThreadAttr_t TJCmsg_Task_attributes = {
  .name = "TJCmsg_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for BTmsg_Task */
osThreadId_t BTmsg_TaskHandle;
const osThreadAttr_t BTmsg_Task_attributes = {
  .name = "BTmsg_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for LCDLED_Task */
osThreadId_t LCDLED_TaskHandle;
const osThreadAttr_t LCDLED_Task_attributes = {
  .name = "LCDLED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void TJCshowTask(void *argument);
void KeyTask(void *argument);
void JoystickTask(void *argument);
void TJCmsgTask(void *argument);
void BTmsgTask(void *argument);
void LCDLEDTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void)
{
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
  to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
  task. It is essential that code added to this hook function never attempts
  to block in any way (for example, call xQueueReceive() with a block time
  specified, or call vTaskDelay()). If the application makes use of the
  vTaskDelete() API function (as this demo application does) then it is also
  important that vApplicationIdleHook() is permitted to return to its calling
  function, because it is the responsibility of the idle task to clean up
  memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
  called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TJCshow_Task */
  TJCshow_TaskHandle = osThreadNew(TJCshowTask, NULL, &TJCshow_Task_attributes);

  /* creation of Key_Task */
  Key_TaskHandle = osThreadNew(KeyTask, NULL, &Key_Task_attributes);

  /* creation of Joystick_Task */
  Joystick_TaskHandle = osThreadNew(JoystickTask, NULL, &Joystick_Task_attributes);

  /* creation of TJCmsg_Task */
  TJCmsg_TaskHandle = osThreadNew(TJCmsgTask, NULL, &TJCmsg_Task_attributes);

  /* creation of BTmsg_Task */
  BTmsg_TaskHandle = osThreadNew(BTmsgTask, NULL, &BTmsg_Task_attributes);

  /* creation of LCDLED_Task */
  LCDLED_TaskHandle = osThreadNew(LCDLEDTask, NULL, &LCDLED_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_TJCshowTask */
/**
 * @brief  Function implementing the TJCshow_Task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_TJCshowTask */
void TJCshowTask(void *argument)
{
  /* USER CODE BEGIN TJCshowTask */
  /* Infinite loop */
  for (;;)
  {
    if (flag.TJCvalfrash)	// 等蓝牙处理数据后发送
    {
      if (flag.RobotMode== 0)
        R1_TJC_val_show();
      else
        R2_TJC_val_show();
     	flag.TJCvalfrash = false;
    }
		if (temp.POWERflashtic++ == 100)	// 检测电量
    {
      powerSend(power_scan());
      temp.POWERflashtic = 0;
    }
    osDelay(40);
  }
  /* USER CODE END TJCshowTask */
}

/* USER CODE BEGIN Header_KeyTask */
/**
 * @brief Function implementing the Key_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_KeyTask */
void KeyTask(void *argument)
{
  /* USER CODE BEGIN KeyTask */
  /* Infinite loop */
  for (;;)
  {
      temp.key_num = KeyScan();      	  
		  temp.ToggleNum = ToggleScan();		
			if (temp.key_num != 0)	// 扫描到按键变化
			{
				if (flag.RobotMode== 0)
					R1_key_function(temp.key_num);
				else
					R2_key_function(temp.key_num);
			}
			if(temp.ToggleNum!=temp.ToggleNumlast)	
			{						
					Toggle_status_show();
					temp.ToggleNumlast=temp.ToggleNum ;
					if (flag.RobotMode== 0)
						R1_toggle_function(temp.ToggleNum);	
					else
						R2_toggle_function(temp.ToggleNum);
			}
			osDelay(30);
  }
  /* USER CODE END KeyTask */
}

/* USER CODE BEGIN Header_JoystickTask */
/**
 * @brief Function implementing the Joystick_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_JoystickTask */
void JoystickTask(void *argument)
{
  /* USER CODE BEGIN JoystickTask */
  /* Infinite loop */
  for (;;)
  { 
    joystickLeft2_scan();
    if (!flag.JSalonemode)
      joystickLeft1_scan();
    joystickRight_scan();
    if (flag.RobotMode == 0)
    {
      r1controlmsg.setx = JScontrolmsg.velX; // 将摇杆控制消息赋值给R1控制消息
      r1controlmsg.sety = JScontrolmsg.velY; 
      r1controlmsg.setw = JScontrolmsg.angW;
    }
    else
    {
      r2controlmsg.Vx = JScontrolmsg.velX; // 将摇杆控制消息赋值给R2控制消息
      r2controlmsg.Vy = JScontrolmsg.velY;
      r2controlmsg.Vw = JScontrolmsg.angW;
    }
    uint8_t *data;		
    if (flag.RobotMode == 0)
       data = dataEncode(R1all_ID, &U2DataFrame); // 编码
    else
       data = dataEncode(R2all_ID, &U2DataFrame); 
		
    enQueueLen(USARTmsgTxQueue, data, U2DataFrame.sendlen); // 入队
    osDelay(25);
  }
  /* USER CODE END JoystickTask */
}

/* USER CODE BEGIN Header_TJCmsgTask */
/**
 * @brief Function implementing the TJCmsg_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TJCmsgTask */
void TJCmsgTask(void *argument)
{
  /* USER CODE BEGIN TJCmsgTask */
  /* Infinite loop */
  for (;;)
  {
    while (deQueue(TJCMsgQueue, (uint8_t *)&TJCrxmsg))
      GetMsg(&tjcMsg, TJCrxmsg, &bw); // 解析触摸屏反馈
    osDelay(40);
  }
  /* USER CODE END TJCmsgTask */
}

/* USER CODE BEGIN Header_BTmsgTask */
/**
 * @brief Function implementing the BTmsg_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_BTmsgTask */
void BTmsgTask(void *argument)
{
  /* USER CODE BEGIN BTmsgTask */
  /* Infinite loop */
  for (;;)
  {
    while (deQueue(USARTmsgRxQueue, (uint8_t *)&BWrxmsg))
      BW16Read(&bw, BWrxmsg);		// 解析蓝牙接收数据
		
    uint8_t _ch;
    int i = 0;
    while (deQueue(USARTmsgTxQueue, &_ch))
    {
      BWtxbuffer[i] = _ch;
      i++;
    }
    if (i != 0 && flag.QUEUEtx)
    {                          
      HAL_UART_Transmit_DMA(&huart2, BWtxbuffer, i); 
    }
    osDelay(40);
  }
  /* USER CODE END BTmsgTask */
}

/* USER CODE BEGIN Header_LCDLEDTask */
/**
 * @brief Function implementing the LCDLED_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_LCDLEDTask */
void LCDLEDTask(void *argument)
{
  /* USER CODE BEGIN LCDLEDTask */
  /* Infinite loop */
  for (;;)
  {
      if (flag.LCDfirstshow) // 第一次LCD刷新界面基本文字要素
      {
				if (flag.RobotMode == 0)
					R1_Interface();
				else 
          R2_Interface();
				Toggle_status_show();
				temp.ToggleNum=0x0F;	
				BTstatus_show();	// 显示一些没用的东西
        flag.LCDfirstshow = false;
      }
      else
        LCD_flash(); // 动态显示
    osDelay(40);
  }
  /* USER CODE END LCDLEDTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

