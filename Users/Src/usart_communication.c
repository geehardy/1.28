#include "bw.h"
#include "usart.h"
#include "msg_queue.h"
#include "tjc.h"
#include "Variables.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if ((huart == &huart2) && flag.QUEUErx)
	{
		enQueue(USARTmsgRxQueue, bwTemp);
		HAL_UART_Receive_DMA(&huart2, (uint8_t *)&bwTemp, 1);
	}
	if (huart == &huart3 && flag.QUEUErx)
	{

		enQueue(TJCMsgQueue, tjcTemp);
		HAL_UART_Receive_DMA(&huart3, (uint8_t *)&tjcTemp, 1);
	}
}
