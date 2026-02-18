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

uint8_t MapState[12];	// 储存2区地图的状态
uint8_t Mapindex=0;		// 2区地图索引，用来标记格子的序号
uint32_t total = 0; 	// 用32位无符号整数暂存24位状态数据，避免溢出

void R1Str_Deal(TjcMsg *tjcMsg)	// 用来处理字符串反馈有数据的情况
{	
	if(tjcMsg->tempData[0]==0x31)	//R1T
		MapState[Mapindex] = 1;
	else if(tjcMsg->tempData[0]==0x32)	//R2T
		MapState[Mapindex] = 2;
	else if(tjcMsg->tempData[0]==0x46)	//False
		MapState[Mapindex] = 3;
	Mapindex++;
}

void GetMsg(TjcMsg *tjcMsg, char msg, BW16 *bw)
{
    if (tjcMsg->getHead)
    { // 得到包头
			if (msg == end[tjcMsg->endIndex]&&(tjcMsg->numendflag ||tjcMsg->head!=num))	// 新增处理num负数反馈时出现并非包尾的FF的情况，click和string不变
        { // 检测包尾
							tjcMsg->endIndex++;
							if (tjcMsg->endIndex == strlen(end))
							{
								tjcMsg->getEnd = true;
								tjcMsg->getHead = false;
								tjcMsg->endIndex = 0;
								if (tjcMsg->head==string && tjcMsg->datalen==0)	// 分支出来处理字符串反馈只有包头包尾没有数据的情况
								{
									MapState[Mapindex]=0;
									Mapindex++;
								}
								if(Mapindex==12)	
								{	// 当状态数组填满时，开始处理状态数组
									R1Map_Deal();
									Mapindex=0;
								}
							}
        }
				else
				    MsgDeal(tjcMsg, msg, bw);
    }
    if (tjcMsg->getEnd)
    {
        if (MsgInHead(msg))  
					{ // 检测包头并储存
            tjcMsg->head = msg;
            tjcMsg->datalen = 0;
            tjcMsg->getEnd = false;
            tjcMsg->getHead = true;
						if(msg ==num)
							tjcMsg->numendflag=false;
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
        BoolChange(LockAngle1, !Boolback(LockAngle1));
        dualswitchvalIndex = 0;
        return;
    case 2:
        BoolChange(LockPoint1, !Boolback(LockPoint1));
        dualswitchvalIndex = 0;
        return;
    case 3:
        BoolChange(ITL1, !Boolback(ITL1));
        dualswitchvalIndex = 0;
        return;
    case 4:
        BoolChange(Zero, !Boolback(Zero));
        dualswitchvalIndex = 0;
        return;
		case 5:
				BoolChange(BeltSwith, !Boolback(BeltSwith));
				dualswitchvalIndex = 0;
				return;
		case 6:
				Press(MatchOK);
				dualswitchvalIndex = 0;
				return;
		case 7:
				Press(TurnBelt);
				dualswitchvalIndex = 0;
				return;
		case 8:
				Press(Refresh);
				dualswitchvalIndex = 0;
				return;
		case 9:
				Press(ReGPS);
				dualswitchvalIndex = 0;
				return;
		case 10:
				Press(SetZero);
				dualswitchvalIndex = 0;
				return;
		case 11:
				Press(ChassisAsk);
				dualswitchvalIndex = 0;
				return;
		case 12:
				Press(WheelAuto);
				dualswitchvalIndex = 0;
				return;
		case 13:
				Press(AutoAct);
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
        memcpy(&r1controlmsg.up, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 5:
    {
        memcpy(&r1controlmsg.tex, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 6:
    {
        memcpy(&r1controlmsg.tey, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 7:
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
    case 1:
    {
        BoolChange(LockAngle2, !Boolback(LockAngle2));
        dualswitchvalIndex = 0;
        return;
    }
    case 2:
    {
        BoolChange (ITL2, !Boolback(ITL2));
        dualswitchvalIndex = 0;
        return;
    }
		case 3:
    {
        BoolChange (RorB, !Boolback(RorB));
        dualswitchvalIndex = 0;
        return;
    }
		case 4:
    {
        Press(LockPoint2);
        dualswitchvalIndex = 0;
        return;
    }
		case 5:
    {
        Press(WeaponPre);
        dualswitchvalIndex = 0;
        return;
    }
		case 6:
    {
        Press(ArmPre2);
        dualswitchvalIndex = 0;
        return;
    }
		case 7:
    {
        Press(WeaponSession);
        dualswitchvalIndex = 0;
        return;
    }
		case 8:
    {
        Press(InstSt);
        dualswitchvalIndex = 0;
        return;
    }
		case 9:
    {
        Press(WheelSt);
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

void R1Map_Deal()
{
	for(int i=0;i<12;i++)
		total |= (uint32_t)MapState[i] << (2 * (11 - i));
	r1controlmsg.Map[0] = (total >> 16) & 0xFF; // 高8位 → 第1个字节		
  r1controlmsg.Map[1] = (total >> 8) & 0xFF;  // 中8位 → 第2个字节		
  r1controlmsg.Map[2] = total & 0xFF;         // 低8位 → 第3个字节		
}

void MsgDeal(TjcMsg *tjcMsg, char msg, BW16 *bw)
{
		tjcMsg->tempData[tjcMsg->datalen] = msg;
		tjcMsg->datalen++;
		if(tjcMsg->head == click && tjcMsg->datalen==3)
    {
				if (tjcMsg->tempData[0] == 0x01 && tjcMsg->tempData[2] == 0x01)	 //R1双态开关反馈处理
				{
					dualswitchvalIndex=tjcMsg->tempData[1];
					R1dualswitchval_deal(tjcMsg);
					return;
				}
				if (tjcMsg->tempData[0] == 0x02 && tjcMsg->tempData[2] == 0x01)	 //R2双态开关反馈处理
				{
					dualswitchvalIndex=tjcMsg->tempData[1];
					R2dualswitchval_deal(tjcMsg);
					return;
				}
    }
    if (tjcMsg->head == string && tjcMsg->datalen==1)	// 如果是字符串反馈
    { 
					R1Str_Deal(tjcMsg);
					return;
    }
    if (tjcMsg->head == num && tjcMsg->datalen==4)	// 如果是数值反馈
    { 
				tjcMsg->numendflag=true;
				if (flag.RobotMode == 0)
				{	
					R1val_Deal(tjcMsg);
					return;
				}
        else
				{
          R2val_Deal(tjcMsg);
					return;
				}
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

void TxtmsgSend(char *msgSend)	// 发送文本控件的文字,需要加双引号
{
    size_t dataLen = strlen(msgSend);
    memset(msgSendTemp, 0, 50);	// 初始化数组
		if(flag.RobotMode==0)
		memcpy(msgSendTemp, "R1.t0.txt=",10);
		else
		memcpy(msgSendTemp, "R2.t0.txt=",10);
		dataLen +=10;
    strcat((char *)msgSendTemp, msgSend);
    msgSendTemp[dataLen] = '\0';
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾
		msgSendTemp[dataLen + strlen(end)] = '\0';
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, dataLen + strlen(end));
}

void powerSend(uint16_t power)	// 应该是作为进度条的数据
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
    valSend(7, r1backmsg.ang0);
    osDelay(5);
    valSend(8, r1backmsg.ang1);
    osDelay(5);
    valSend(9, r1backmsg.ang2);
    osDelay(5);
    valSend(10, r1backmsg.ang3);
    osDelay(5);
}
void R2_TJC_val_show()
{
		valSend(6, r2backmsg.FL);
    osDelay(5);
    valSend(7, r2backmsg.FR);
    osDelay(5);
    valSend(8, r2backmsg.BL);
    osDelay(5);
    valSend(9, r2backmsg.BR);
    osDelay(5);
		valSend(10, r2backmsg.grid);
    osDelay(5);
    valSend(11, r2backmsg.upperarm);
    osDelay(5);
    valSend(12, r2backmsg.takehead);
    osDelay(5);
    valSend(13, r2backmsg.wheelstate);
    osDelay(5);
}