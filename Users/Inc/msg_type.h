#ifndef MSG_TYPE_H
#define MSG_TYPE_H

#include "main.h"
#include "stdbool.h"

// 数据包ID 消息类型
typedef enum
{
    R1all_ID = 0x01,
    R2all_ID = 0x02,
} MsgId_E;

#define R1BoolNum 35
#define R2BoolNum 25

typedef enum
{
    /*********R1  bool值 */
		Zero, 				//回零1
		Protect, 			//保护
		Enable1, 			//使能
		ChassisAsk,		//底盘询问
		WheelAuto,		//底盘自动
		Reset1,				//R1复位
		LockPoint1,		//R1锁点
		ITL1,					//R1叉锁
	
		ArmEn,				//臂使能2
		TakeRod,			//取杆
		Match,				//对接
		ArmReset,			//臂复位
		Attack,				//攻击
		ArmLose,			//丢臂
		ScrEn,				//Scr使能
		TakeCube1,		//R1取方块
	
		PutCube1,			//R1放方块3
		PassCube,			//递方块
		ScrReset,			//Scr复位
		ScrReget,			//Scr收回
		LiftMode,			//抬升模式
		AutoAct,			//
		ArmPre1,			//R1臂准备
		LiftUp1,			//R1抬升
		
		ReachOut,			//伸出4
		TurnBelt,			//转摩擦带
		WeaponStand,	//武器直立
		TakeCubeMode,	//取块模式
		PassCubeMode,	//递块模式
		BeltSwith,		//摩擦带正反
		LockAngle1,		//R1锁角
		ReGPS,				//重定位
		
		SetZero,			//置零5
		MatchOK,			//对接成功
		Refresh,			//刷新
		
    /*********R2  bool值 */
		WheelEn2, 		//R2底盘使能1
		WheelSt,			//底盘启动
		InstEn,				//机构使能
		InstSt,				//机构启动
		LockAngle2,		//R2锁角度
		LockPoint2,		//R2锁点
		Reset2,				//R2复位
		LiftUp2,			//R2抬升
		
		LiftDown,			//下降2
		WeaponSession,//武器头场次
		TakeHead,			//取武器头
		WeaponPre,		//武器准备
		FreeHead,			//武器头释放
		ArmPre2,			//R2臂准备
		TakeCube2,		//R2取方块
		TakeCubeHL,		//取方块高低
		
		CarryCube,		//载方块3
		PutCube2,			//R2放方块
		ReadCube,			//读取方块
		ClearPath,		//清空路径
		Cell,					//Cell
		ITL2,					//R2叉锁
		ActiveSet,		//主动置位
		SendPath,			//发送路径
		
		RorB,					//红蓝场4
		StairHL,			//高低台阶
		NextStep,			//下一步
} ConBool;

#pragma pack(1)		//以CPU运行效率为代价，修改结构体对齐数
typedef struct R1ControlMsg_S
{
    uint8_t BOOL[R1BoolNum/8+1]; 
    uint8_t area; 	//所在区域
    uint8_t rod; 		//杆号
		short angle; 		//角度
		short tex; 			//
		short tey; 			//
		short setx;
		short sety;
		short setw;
		short layer;		//层
		short up;				//升
    int pilenum1;	//0~6桩
    int pilenum2;	//7~12桩
} R1ControlMsg_S;

typedef struct R1BackMsg_S
{
    short ang0;
    short ang1;
    short ang2;
    short ang3;
    float disX;
    float disY;
    float Yaw;
    float pitch;  
} R1BackMsg_S;

typedef struct R2ControlMsg_S
{
		uint8_t BOOL[R2BoolNum/8 +1];
		uint8_t mode;			//
		uint8_t gridstate;//格子状态
		uint8_t headnum;	//武器头序号
		short Vx;     
		short Vy;
		short Vw;
		short X;
		short Y;
		short angle;
} R2ControlMsg_S;

typedef struct R2BackMsg_S
{
		uint8_t grid;				//格子
		uint8_t upperarm;		//大臂
		uint8_t takehead;		//取头
		uint8_t wheelstate;	//底盘状态
		short X;
		short Y;
		short FL;
		short FR;
		short BR;
		short BL;
		float angle;
} R2BackMsg_S;

#pragma pack()

void BoolChange(ConBool Type, bool value);
bool Boolback(ConBool Type);	//返回指定位的bool值

void Press(ConBool type);

int MsgLen(MsgId_E msgType);
int MsgAddr(MsgId_E msgType);

extern R1ControlMsg_S r1controlmsg;
extern R1BackMsg_S r1backmsg;
extern R2ControlMsg_S r2controlmsg;
extern R2BackMsg_S r2backmsg;

#endif