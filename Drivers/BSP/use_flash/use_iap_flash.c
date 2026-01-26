/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "use_iap_flash.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Error code */
enum
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR
};

enum
{
  FLASHIF_PROTECTION_NONE = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED = 0x2,
  FLASHIF_PROTECTION_RDPENABLED = 0x4,
};

/* Private define ------------------------------------------------------------*/
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)  /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08004000)  /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08008000)  /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x0800C000)  /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08010000)  /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5 ((uint32_t)0x08020000)  /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6 ((uint32_t)0x08040000)  /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7 ((uint32_t)0x08060000)  /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8 ((uint32_t)0x08080000)  /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9 ((uint32_t)0x080A0000)  /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10 ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11 ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t Address);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 解锁闪存进行写入访问
 *
 */
void FLASH_If_Init(void)
{
  HAL_FLASH_Unlock();

  /* Clear pending flags (if any) */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
}

/**
 * @brief 此功能用于擦除所有APP的闪存区域
 *
 * @param StartSector 闪存区的开始地址
 * @param EndSector 闪存区的结束地址
 * @return uint32_t 0: 已成功擦除用户闪存区域
 *                  1: 发生错误
 */
uint32_t FLASH_If_Erase(uint32_t StartSector, uint32_t EndSector)
{
  uint32_t SectorError;
  FLASH_EraseInitTypeDef pEraseInit;

  /* 解锁flash,以启用flash控制寄存器访问 */
  FLASH_If_Init();

  pEraseInit.TypeErase = TYPEERASE_SECTORS;
  pEraseInit.Sector = GetSector(StartSector);
  pEraseInit.NbSectors = GetSector(EndSector) - pEraseInit.Sector + 1;
  pEraseInit.VoltageRange = VOLTAGE_RANGE_3;

  if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK)
  {
    /* 页面擦除时出错 */
    return (1);
  }
  return (0);
}
/**
 * @brief 此函数在flash中写入一个数据缓冲区（数据是32位对齐的）
 *        注:写入数据缓冲区后，检查闪存内容。
 * @param FlashAddress 写入数据缓冲区的起始地址
 * @param Data 数据缓冲区上的指针
 * @param DataLength 数据缓冲区长度（单位为32位字）
 * @return uint32_t 0:数据成功写入闪存
 *                   2:闪存中写入的数据与预期的不同
 *                   3:在闪存中写入数据时出错
 */
uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t *Data, uint32_t DataLength)
{
  for (uint32_t i = 0; i < DataLength; i++)
  {
    /* 器件电压范围假定为[2.7V到3.6V]，操作将由字来完成 */
    if (HAL_FLASH_Program(TYPEPROGRAM_WORD, FlashAddress, *(uint32_t *)(Data + i)) == HAL_OK)
    {
      /*检查写入值*/
      if (*(uint32_t *)FlashAddress != *(uint32_t *)(Data + i))
      {
        /*闪存内容与SRAM内容不匹配*/
        return (FLASHIF_WRITINGCTRL_ERROR);
      }
      /*增量闪存目标地址*/
      FlashAddress += 4;
    }
    else
    {
      /*在闪存中写入数据时出错*/
      return (FLASHIF_WRITING_ERROR);
    }
  }
  /* 数据成功写入闪存 */
  return (FLASHIF_OK);
}

/**
 * @brief Get the Sector object 获取给定地址的扇区
 *
 * @param Address 目标地址
 * @return uint32_t 给定地址的扇区
 */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector;

  if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_7;
  }
  else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_8;
  }
  else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_9;
  }
  else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_10;
  }
  else
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}
