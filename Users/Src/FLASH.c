#include "FLASH.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
u32 PAGEError = 0;
/**********************************************************************************
 * 函数功能: 扇形擦除
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明：无
 */
void Flash_Erase(void)
{
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;  // 只擦除扇形区
    EraseInitStruct.Sector = FLASH_USER_CHOOSE_SECTORS;   // 从第11扇区开始擦除
    EraseInitStruct.NbSectors = 1;                        // 擦除的扇区数
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 电压范围3(2.7V-3.6V)
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        Error_Handler();
    }
}
/**********************************************************************************
 * 函数功能: 数据写入
 * 输入参数: 写入数据缓存数组指针、写入数据数
 * 返 回 值: 无
 * 说    明：无
 */
void Flash_Write_Byte(u8 *pBuffer, u16 NumToWrite)
{

    u16 i = 0;
    u32 Address = FLASH_USER_START_ADDR;
    HAL_FLASH_Unlock(); // 解锁
    Flash_Erase();      // 先擦除 再写入
    HAL_Delay(100);
    while ((Address < FLASH_USER_END_ADDR) && (i < NumToWrite))
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Address, (uint64_t)pBuffer[i]) == HAL_OK)
        {
            Address++; // 地址后移1个字节
            i++;
        }
        else
        {
            Error_Handler();
        }
    }

    HAL_FLASH_Lock(); // 上锁
}

/**********************************************************************************
 * 函数功能: 数据读取
 * 输入参数: 读取数据缓存数组指针、读出数据数
 * 返 回 值: 无
 * 说    明：无
 */
void Flash_Read_Byte(u8 *pBuffer, u16 NumToRead)
{
    u16 i = 0;
    u32 Address = FLASH_USER_START_ADDR;

    while ((Address < FLASH_USER_END_ADDR) && (i < NumToRead))
    {
        pBuffer[i++] = *(u8 *)Address;
        Address++; // 地址后移1个字节
    }
}
