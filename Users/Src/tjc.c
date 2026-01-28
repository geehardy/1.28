#include "tjc.h"
#include "usart.h"
#include "msg_queue.h"
#include "string.h"
#include "keyboard.h"
#include "msg_type.h"
#include "cmsis_os2.h"
#include "Variables.h"
#include "Start.h"

char tjcTemp;
Queue *TJCMsgQueue;
//Queue *TJCTxQueue;
TjcMsg tjcMsg = {.getEnd= true ,.getHead= false ,.head= 0 ,.endIndex=0 ,.tempData= {0} };
uint8_t msgSendTemp[50];

bool TJCInit()
{
    TJCMsgQueue = newQueue();
    // TJCTxQueue = newQueue();
    if (TJCMsgQueue == NULL)
        return false;
    HAL_UART_Receive_DMA(&huart3, (uint8_t *)&tjcTemp, 1); // 打开接收回调
    return true;
};

void GetMsg(TjcMsg *tjcMsg, char msg, BW16 *bw)
{
    if (tjcMsg->getHead)
    { // 得到包头
        if (msg == end[tjcMsg->endIndex])
        { // 检测包尾
            tjcMsg->endIndex++;
            if (tjcMsg->endIndex == strlen(end))
            {
                tjcMsg->getEnd = true;
                tjcMsg->getHead = false;
                tjcMsg->endIndex = 0;
						}
        }
				else
				    MsgDeal(tjcMsg, msg, bw);
    }
    if (tjcMsg->getEnd)
    {
        if (MsgInHead(msg))  
					{ // 检测包头，	TJC的包头只有一个字节，所以一次就判断出来，只不过要储存具体的值
            tjcMsg->head = msg;
            tjcMsg->datalen = 0;
            tjcMsg->getEnd = false;
            tjcMsg->getHead = true;
        }
    }
}

// 检测包头
bool MsgInHead(char msg)
{
    for (size_t i = 0; i < num_valid_value; i++)
    {
        if (valid_value[i] == msg)
        {
            return true;
        }
    }
    return false;
}
// 双态开关和数值反馈处理
uint8_t dualswitchvalIndex = 0; // 双控开关反馈索引
uint8_t msgbackIndex = 1;

void R1dualswitchval_deal(TjcMsg *tjcMsg)
{
	    // 双态开关val接收
    switch (dualswitchvalIndex)	//
    {
    case 1:
        BoolChange(ChassisAsk, !Boolback(ChassisAsk));
        dualswitchvalIndex = 0;
        return;
    case 2:
        BoolChange(LockAngle1, !Boolback(LockAngle1));
        dualswitchvalIndex = 0;
        return;
    case 3:
        BoolChange(LockPoint1, !Boolback(LockPoint1));
        dualswitchvalIndex = 0;
        return;
    case 4:
        BoolChange(ITL1, !Boolback(ITL1));
        dualswitchvalIndex = 0;
        return;
		case 5:
				BoolChange(MatchOK, !Boolback(MatchOK));
				dualswitchvalIndex = 0;
				return;
		case 6:
				BoolChange (BeltSwith, !Boolback(BeltSwith));
				dualswitchvalIndex = 0;
				return;
		case 7:
				BoolChange (TurnBelt, !Boolback(TurnBelt));
				dualswitchvalIndex = 0;
				return;
		case 8:
				BoolChange (LiftMode, !Boolback(LiftMode));
				dualswitchvalIndex = 0;
				return;
		case 9:
				BoolChange (TakeCubeMode, !Boolback(TakeCubeMode));
				dualswitchvalIndex = 0;
				return;
		case 10:
				BoolChange (PassCubeMode, !Boolback(PassCubeMode));
				dualswitchvalIndex = 0;
				return;
		case 11:
				BoolChange (WheelAuto, !Boolback(WheelAuto));
				dualswitchvalIndex = 0;
				return;
		case 12:
				BoolChange (AutoAct, !Boolback(AutoAct));
				dualswitchvalIndex = 0;
				return;
		case 13:
				BoolChange (Refresh, !Boolback(Refresh));
				dualswitchvalIndex = 0;
				return;
    default:
        break;
    }
}

void R1val_Deal(TjcMsg *tjcMsg)
{

    // 数值输入接收
    switch (msgbackIndex)
    {
    case 1:
    {
        memcpy(&r1controlmsg.area, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
    case 2:
    {
        memcpy(&r1controlmsg.rod, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
    case 3:
    {
        memcpy(&r1controlmsg.layer, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 4:
    {
      if(tjcMsg->tempData[0]<7)
			{
				memcpy(&r1controlmsg.pilenum1, tjcMsg->tempData, 4);
				r1controlmsg.pilenum2=0;
			}
			else
			{
				memcpy(&r1controlmsg.pilenum2, tjcMsg->tempData, 4);
				r1controlmsg.pilenum1=0;
			}
      msgbackIndex++;
      break;
    }
    case 5:
    {
        memcpy(&r1controlmsg.up, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 6:
    {
        memcpy(&r1controlmsg.tex, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 7:
    {
        memcpy(&r1controlmsg.tey, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 8:
    {
        memcpy(&r1controlmsg.angle, tjcMsg->tempData, 2);
        msgbackIndex =1;
        break;
    }
		default:
				break;
	}
}

void R2dualswitchval_deal(TjcMsg *tjcMsg)
{
	    // 双态开关val接收
    switch (dualswitchvalIndex)
    {
    case 14:
    {
        BoolChange (InstSt, !Boolback(InstSt));
        dualswitchvalIndex = 0;
				osDelay(5);
        return;
    }
    case 15:
    {
        BoolChange (WheelSt, !Boolback(WheelSt));
        dualswitchvalIndex = 0;
        return;
    }
		case 16:
    {
        BoolChange (WeaponSession, !Boolback(WeaponSession));
        dualswitchvalIndex = 0;
				osDelay(5);
        return;
    }
		case 17:
    {
        BoolChange (LockAngle2, !Boolback(LockAngle2));
        dualswitchvalIndex = 0;
        return;
    }
		case 18:
    {
        BoolChange(ITL2, !Boolback(ITL2));
        dualswitchvalIndex = 0;
        return;
    }
		case 19:
    {
        BoolChange(WeaponPre, !Boolback(WeaponPre));
        dualswitchvalIndex = 0;
        return;
    }
		case 20:
    {
        BoolChange(ArmPre2, !Boolback(ArmPre2));
        dualswitchvalIndex = 0;
        return;
    }
		case 21:
    {
        BoolChange(LockPoint2, !Boolback(LockPoint2));
        dualswitchvalIndex = 0;
        return;
    }
		case 22:
    {
        BoolChange(RorB, !Boolback(RorB));
        dualswitchvalIndex = 0;
        return;
    }
    default:
        break;
    }
}
void R2val_Deal(TjcMsg *tjcMsg)
{

    // 数值输入接收
    switch (msgbackIndex)
    {
    case 1:
    {
        memcpy(&r2controlmsg.X, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 2:
    {
        memcpy(&r2controlmsg.Y, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 3:
    {
        memcpy(&r2controlmsg.angle, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 4:
    {
//        int32_t temp;
//        memcpy(&temp, tjcMsg->tempData, 4);
//        r2controlmsg.Z_angle = (float)temp / 10000;
				memcpy(&r2controlmsg.gridstate, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
    case 5:
    {
//			 int32_t temp;
//        memcpy(&temp, tjcMsg->tempData, 4);
//        r2controlmsg.Go_angle= (float)temp / 10000;
//        msgbackIndex = 1;
				memcpy(&r2controlmsg.headnum, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
		case 6:
		{
				memcpy(&r2controlmsg.mode, tjcMsg->tempData, 1);
        msgbackIndex =1;
				break;
		}
    default:
//        msgbackIndex = 1;
        break;
    }
}

void MsgDeal(TjcMsg *tjcMsg, char msg, BW16 *bw)
{
		tjcMsg->tempData[tjcMsg->datalen] = msg;
		tjcMsg->datalen++;
		if(tjcMsg->head == click && tjcMsg->datalen==3)
    {
						//R1双态开关反馈处理
            if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x01 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 1;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
            if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x02 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 2;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x03 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 3;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x04 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 4;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x05 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 5;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x06 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 6;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x07 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 7;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x08 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 8;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x09 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 9;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x0A && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 10;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x0B && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 11;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x0C && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 12;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[1] == 0x0D && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 13;
								R1dualswitchval_deal(tjcMsg);
                return;
            }
						
						//R2双态开关反馈处理
            if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x07 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 14;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
            if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x08 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 15;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
            if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x09 && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 16;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
            if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0A && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 17;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0B && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 18;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0C && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 19;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0D && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 20;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0E && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 21;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
						if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[1] == 0x0F && tjcMsg->tempData[2] == 0x01)
            {
                dualswitchvalIndex = 22;
								R2dualswitchval_deal(tjcMsg);
                return;
            }
        }
//        if (tjcMsg->head == string && tjcMsg->getEnd)	// 如果是字符串反馈
//        { 
//            bw->nameLen = tjcMsg->datalen - strlen(end);
//            memcpy(&bw->name, &tjcMsg->tempData, bw->nameLen);
//            return;
//        }
        if (tjcMsg->head == num && tjcMsg->datalen==4)	// 如果是数值反馈
        { 
            if (flag.RobotMode == 0)
                R1val_Deal(tjcMsg);
            else
                R2val_Deal(tjcMsg);
        }
}
void MsgSend(char *msgSend)
{
    size_t dataLen = strlen(msgSend);
    memset(msgSendTemp, 0, 50);
    memcpy(msgSendTemp, msgSend, dataLen);
    msgSendTemp[dataLen] = '\0';
    msgSendTemp[dataLen + strlen(end)] = '\0';
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, dataLen + strlen(end));
}
void powerSend(uint8_t power)	// 应该是作为进度条的数据
{

    memset(msgSendTemp, 0, 50);
    memcpy(msgSendTemp, "Power.val=", 10);
    msgSendTemp[10] = power / 10 + '0';
    msgSendTemp[11] = power % 10 + '0';
    msgSendTemp[12] = '\0';
    msgSendTemp[12 + strlen(end)] = '\0';
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, 12 + strlen(end));
}

void valSend(uint8_t num, int16_t val)
{
    uint8_t datalen = 0;
    memset(msgSendTemp, 0, 50);
    msgSendTemp[0] = 'n';
    if (num < 10)
    {
        msgSendTemp[1] = num + '0';
    }
    else
    {
        msgSendTemp[1] = num / 10 + '0';
        msgSendTemp[2] = num % 10 + '0';
    }
    strcat((char *)msgSendTemp, ".val=");
    osDelay(10);
    if (val < 0)
    {
        strcat((char *)msgSendTemp, "-");
        val = -val; // 取绝对值
    }
    datalen = strlen((char *)msgSendTemp);
    msgSendTemp[datalen++] = val / 10000 + '0';
    msgSendTemp[datalen++] = (val % 10000) / 1000 + '0';
    msgSendTemp[datalen++] = (val % 1000) / 100 + '0';
    msgSendTemp[datalen++] = (val % 100) / 10 + '0';
    msgSendTemp[datalen++] = val % 10 + '0';
    msgSendTemp[datalen] = '\0';
    msgSendTemp[datalen + strlen(end)] = '\0';
    osDelay(10);
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾

    //    enQueueLen(TJCTxQueue, msgSendTemp, datalen + strlen(end));
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, datalen + strlen(end));
}
void float_valsend(uint8_t num, float val)
{
    memset(msgSendTemp, 0, 50);
    msgSendTemp[0] = 't';
    msgSendTemp[1] = '0' + num;
    strcat((char *)msgSendTemp, ".txt=\"");
    if (val < 0)
    {
        strcat((char *)msgSendTemp, "-");
        val = -val; // 取绝对值
    }
    uint8_t datalen = strlen((char *)msgSendTemp);
    int16_t intPart = (int16_t)val;                        // 整数部分
    int16_t fracPart = (int16_t)((val - intPart) * 10000); // 小数部分，保留四位小数
    if (intPart < 10)
    {
        msgSendTemp[datalen++] = intPart + '0';
    }
    else
    {
        msgSendTemp[datalen++] = intPart / 100 + '0';
        msgSendTemp[datalen++] = (intPart % 100) / 10 + '0';
        msgSendTemp[datalen++] = intPart % 10 + '0';
    }
    msgSendTemp[datalen++] = '.';
    msgSendTemp[datalen++] = fracPart / 1000 + '0';
    msgSendTemp[datalen++] = (fracPart % 1000) / 100 + '0';
    msgSendTemp[datalen++] = (fracPart % 100) / 10 + '0';
    msgSendTemp[datalen++] = fracPart % 10 + '0';
    strcat((char *)msgSendTemp, "\"");
    osDelay(10);
    MsgSend((char *)msgSendTemp);
}

void R1_TJC_val_show()
{
    valSend(8, r1backmsg.ang0);
    osDelay(10);
    valSend(9, r1backmsg.ang1);
    osDelay(10);
    valSend(10, r1backmsg.ang2);
    osDelay(10);
    valSend(11, r1backmsg.ang3);
    osDelay(10);
}
void R2_TJC_val_show()
{
		valSend(6, r2backmsg.FL);
    osDelay(10);
    valSend(7, r2backmsg.FR);
    osDelay(10);
    valSend(8, r2backmsg.BL);
    osDelay(10);
    valSend(9, r2backmsg.BR);
    osDelay(10);
		valSend(10, r2backmsg.grid);
    osDelay(10);
    valSend(11, r2backmsg.upperarm);
    osDelay(10);
    valSend(12, r2backmsg.takehead);
    osDelay(10);
    valSend(13, r2backmsg.wheelstate);
    osDelay(10);
}