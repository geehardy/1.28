#include "bw.h"
#include "cmsis_os2.h"
#include "string.h"
#include "tjc.h"
#include "usart.h"
#include "msg_queue.h"
#include "msg_type.h"
#include "data_process.h"
#include "Variables.h"

/*
BW16模块
send 指令后加\r\n                            如AT\r\n
receive 格式  发送的指令加\r\n加响应加\r\n      如AT\r\n\r\nOK\r\n
**用蓝牙从机模式调试时注意**
UUID:55535343fe7d4ae58fa99fafd205e455
TXUUID:49535343884143f4a8d4ecbe34729bb3
RXUUID:495353431e4d4bd9ba6123c647249616
蓝牙名称bw16-BTmode
*/

BW16 bw = {.CheckBite=0,.getEnd = true, .getHead = false, .endIndex = 0,.headIndex = 0,.redatalen=0,.connect=false,.init=false,.tempData=0};

char bwTemp;

// bw16初始化
bool BW16Init(BW16 *bw)
{
    // 队列初始化
    USARTmsgRxQueue = newQueue(); // 串口接收队列
    USARTmsgTxQueue = newQueue(); // 串口发送队列
    if (USARTmsgRxQueue == NULL || USARTmsgTxQueue == NULL) // 内存申请失败
        return false;
		HAL_UART_Receive_DMA(&huart2,(uint8_t *)&bwTemp,1);
		return true;
}

/***********************************BTmsgBack deal User Define Begin********************************/
// 透传数据处理
void R1_BWtranMsgDeal(BW16 *bw, char msg)
{ // R1接收数据处理
    bw->tempData[bw->redatalen] = msg;
    bw->redatalen++;
    if (bw->redatalen==25)   		
    {
        if(bw->CheckBite==bw->tempData[24])	// 校验位正确
				{
				memcpy(&r1backmsg.ang0, &bw->tempData[0], 2);
        memcpy(&r1backmsg.ang1, &bw->tempData[2], 2);
        memcpy(&r1backmsg.ang2, &bw->tempData[4], 2);
        memcpy(&r1backmsg.ang3, &bw->tempData[6], 2);
        memcpy(&r1backmsg.disX, &bw->tempData[8], 4);
        memcpy(&r1backmsg.disY, &bw->tempData[12], 4);
        memcpy(&r1backmsg.Yaw, &bw->tempData[16], 4);
        memcpy(&r1backmsg.pitch, &bw->tempData[20], 4);					
        flag.TJCvalfrash = true;
				}
				bw->redatalen = 0;
				bw->CheckBite = 0;
				osDelay(5);	//防止被优化
    }
		else
		{
			bw->CheckBite+= msg;
		}
}
void R2_BWtranMsgDeal(BW16 *bw, char msg)
{ // R2接收数据处理
		bw->tempData[bw->redatalen] = msg;
		bw->redatalen++;				
    if (bw->redatalen==21)   		
    {		
			if(bw->CheckBite==bw->tempData[20])	//判断求和校验位
			{
        memcpy(&r2backmsg.grid, &bw->tempData[0], 1);
        memcpy(&r2backmsg.upperarm, &bw->tempData[1], 1);
        memcpy(&r2backmsg.takehead, &bw->tempData[2], 1);
        memcpy(&r2backmsg.wheelstate, &bw->tempData[3], 1);
        memcpy(&r2backmsg.X, &bw->tempData[4], 2);
        memcpy(&r2backmsg.Y, &bw->tempData[6], 2);
        memcpy(&r2backmsg.FL, &bw->tempData[8], 2);
        memcpy(&r2backmsg.FR, &bw->tempData[10], 2);
        memcpy(&r2backmsg.BR, &bw->tempData[12], 2);
        memcpy(&r2backmsg.BL, &bw->tempData[14], 2);
				memcpy(&r2backmsg.angle, &bw->tempData[16], 4);
        flag.TJCvalfrash = true;
			}
			bw->redatalen = 0;
			bw->CheckBite = 0;
			osDelay(5);	//防止被优化
    }
		else
			bw->CheckBite+= msg;
}

/***********************************BTmsgBack deal User Define end********************************/
void BW16Read(BW16 *bw, char msg)
{
        if (bw->getHead)
        { // 得到包头
            if (msg == (uint8_t)U2DataFrame.tail[bw->endIndex])
            { // 检测包尾
                bw->endIndex++;
                if (bw->endIndex == U2DataFrame.taillen)
                {
                    bw->getEnd = true;
                    bw->getHead = false;
                    bw->endIndex = 0;
                }
            }
            else 
						{
							if (flag.RobotMode==1)
                R2_BWtranMsgDeal(bw, msg);
              else
                R1_BWtranMsgDeal(bw, msg);
						}
        }
        else if (msg == (uint8_t)U2DataFrame.head[bw->headIndex])
        {
            bw->headIndex++;
            if (bw->headIndex == U2DataFrame.headlen)
            {
                bw->getHead = true;
                bw->getEnd = false;
                bw->headIndex = 0;
            }
						return;
        }
}