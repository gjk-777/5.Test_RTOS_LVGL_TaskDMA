/**
 ****************************************************************************************************
 * @file        sram.c
 * @author      K-AI
 * @version     V1.0
 * @date        2025-10-05
 * @brief       外部SRAM 驱动代码
 ****************************************************************************************************
 * @attention
FSMC控制器存储空间信息：- 分为4个Bank，每个Bank 256MB
                      - Bank 1地址范围：0x60000000~0x6FFFFFFF
                      - Bank 1用途：连接SRAM、NOR FLASH、PSRAM、TFT LCD
- Bank 1子区划分（每个64MB，26位地址线）：
  - Bank 1子区1：0x60000000~0x63FFFFFF
  - Bank 1子区2：0x64000000~0x67FFFFFF
  - Bank 1子区3：0x68000000~0x6BFFFFFF（开发板用于外扩SRAM）-----1024kb---国产XM8A51216
  - Bank 1子区4：0x6C000000~0x6FFFFFFF（开发板用于连接TFT LCD）
 ****************************************************************************************************
 */

#include "sram.h"
#include "fsmc.h"
extern SRAM_HandleTypeDef hsram3;
/**
 * @brief       往SRAM指定地址写入指定长度数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写入的地址(最大32bit)
 * @param       datalen : 要写入的字节数(最大32bit)
 * @retval      无
 */
void SRAM_Write_Direct(uint32_t *pBuffer, uint32_t *pAddress, uint32_t BufferSize)
{
    pAddress += SRAM_ADDR_Begin;
    HAL_SRAM_Write_32b(&hsram3, pAddress, pBuffer, BufferSize);
}
// 往SRAM指定地址写入指定长度数据(使用DMA)
void SRAM_Write_DMA(uint32_t *pBuffer, uint32_t *pAddress, uint32_t BufferSize)
{
    pAddress += SRAM_ADDR_Begin;
    HAL_SRAM_Write_DMA(&hsram3, pAddress, pBuffer, BufferSize);
}
/**
 * @brief       从SRAM读取数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 读取的起始地址(最大32bit)
 * @param       datalen : 要读取的字节数(最大32bit)
 * @retval      无
 */
void SRAM_Read_Direct(uint32_t *pBuffer, uint32_t *pAddress, uint32_t BufferSize)
{
    pAddress += SRAM_ADDR_Begin;
    HAL_SRAM_Read_32b(&hsram3, pAddress, pBuffer, BufferSize);
}
// 从SRAM指定地址读取指定长度数据(使用DMA)
void SRAM_Read_DMA(uint32_t *pBuffer, uint32_t *pAddress, uint32_t BufferSize)
{
    pAddress += SRAM_ADDR_Begin;
    HAL_SRAM_Read_DMA(&hsram3, pAddress, pBuffer, BufferSize);
}
