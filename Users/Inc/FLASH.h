#ifndef _FLASH_H_
#define _FLASH_H_
#include "main.h"
#include "stm32f4xx_hal_flash_ex.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
                                                                                                                                       // 用户用到的扇区数                                                                                 // 扇区大小为16K字节
#define STM32_FLASH_BASE 0x08000000  
#define FLASH_USER_CHOOSE_SECTORS FLASH_SECTOR_11// 用户使用扇区
#define FLASH_USER_CHOOSE_SECTORS_SIZE 128*1024                                 // 选择扇区大小
#define FLASH_USER_START_ADDR 0x080E0000                  // 开始地址
#define FLASH_USER_END_ADDR 0x080FFFFF                  // 结束地址


void Flash_Erase(void);
void Flash_Write_Byte(u8 *pBuffer, u16 NumToWrite);
void Flash_Read_Byte(u8 *pBuffer, u16 NumToRead);

#endif
