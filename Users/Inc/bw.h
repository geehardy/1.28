#ifndef __BW_H
#define __BW_H

#include "main.h"
#include "stdbool.h"

#define MAX_SCAN_BT_NUM 48

//#define AT_HEAD "AT"
//#define AT_TAIL "\r\nOK\r\n"
//#define EVENT_HEAD "+EVENT"
//#define WAIT_HEAD "+BLEAUTOCON:"

//#define R1_BTMAC "2442e3060495"
//#define R2_BTMAC "94c960fbe2d3"



typedef struct BW16
{
    bool connect;
    bool init;
    bool getHead;
		bool getEnd;
		uint8_t headIndex;
    uint8_t endIndex;
    uint8_t redatalen;
    char tempData[96];
		uint8_t CheckBite;
} BW16;

bool BW16Init(BW16 *bw);

void BW16Read(BW16 *bw,char msg);

extern BW16 bw;
extern char bwTemp;

#endif