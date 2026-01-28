#ifndef _Variables_H
#define _Variables_H

#include "stdbool.h"
#include "main.h"

typedef struct Flag_S
{
    bool RobotMode; // 0为R1，1为R2

    bool QUEUErx; // 队列接收标志
    bool QUEUEtx; // 队列发送标志
    bool LCDfirstshow; // LCD第一次显示标志
    bool JSfast_slow; // 摇杆快慢切换标志
		bool JSfast_slow_last;
    bool JSalonemode; // 单摇杆模式标志
		bool JSalonemode_last;
    bool TJCvalfrash; // 串口屏数据刷新标志
} Flag_S;

typedef struct Temp_S
{
    uint8_t POWERflashtic; // 电源刷新计数
		uint8_t key_num;
		uint8_t ToggleNum;
		uint8_t ToggleNumlast;	// 用来储存改变拨码开关的序号用于tft显示
    short L1xlast;
    short L1ylast;
    short Rxlast;
    short Rylast;
    short L1xNow;
    short L1yNow;
    short RxNow;
    short RyNow;

} Temp_S;

extern  Temp_S temp;  // 全局临时变量
extern Flag_S flag;  // 全局标志位
void FlagInit(void); // 初始化标志位
void TempInit(void); // 初始化临时变量
#endif