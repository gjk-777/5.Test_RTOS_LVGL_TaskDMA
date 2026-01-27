#include "mysfud.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SPIFlash_Hal.h"

#define SFUD_CMD_READ_DATA 0x03         /// SFUD命令-读取数据
#define SFUD_CMD_READ_STATUS 0x05       /// SFUD命令-读取状态
#define SFUD_CMD_READ_ID 0x9F           /// SFUD命令-读取ID
#define SFUD_CMD_SET_WRITE_ENABLE 0x06  /// SFUD命令-设定写入
#define SFUD_CMD_SET_WRITE_DISABLE 0x04 /// SFUD命令-关闭写入
#define SFUD_CMD_ERASE_SECTOR 0x20      /// SFUD命令-擦除扇区
#define SFUD_CMD_ERASE_BLOCK 0xD8       /// SFUD命令-擦除块
#define SFUD_CMD_PROGRAM_256B 0x02      /// SFUD命令-字节编程 1-256B

#define SFUD_MAX_ADDRESS 0x7FFFFF /// 最大地址
#define SFUD_WRITE_MAX (256)      /// 单次写入最大单位 256B

#define SFUD_WAIT_WRITE_LIMIT_100US (4)              /// 写入数据等待限制 4ms
#define SFUD_WAIT_ERASE_BLOCK_LIMIT_100US (1000 * 3) /// 块擦除等待限制 3s
#define SFUD_WAIT_ERASE_SECTOR_LIMIT_100US (300)     /// 扇区擦除等待限制 300ms

typedef enum edef_EraseType /// FLASH擦除类型
{
  ERASE_SECTOR = 0, ///< 擦除扇区
  ERASE_BLOCK = 1,  ///< 擦除块
} e_EraseType;

#pragma pack(1)
typedef struct tdef_StatusReg /// 状态寄存器
{
  uint8_t writeInProgress : 1; ///< 写入操作进行中
  uint8_t writeEnable : 1;     ///< 写使能
  uint8_t blockProtection : 4; ///< 块保护
  uint8_t : 1;
  uint8_t statusRegWriteProtect : 1; ///< 状态寄存器写保护
} t_StatusReg;
#pragma pack()

static bool SFUD_ReadStatus(t_StatusReg *status);
static bool SFUD_WriteEnable(bool enable);
static bool SFUD_Erase(e_EraseType type, uint32_t addr);
static bool SFUD_WaitBusy(uint32_t maxWaitCycle);

static bool prvProgram_Single_page(uint32_t addr, uint16_t size, const uint8_t *data)
{
  /* 写使能 */
  if (!SFUD_WriteEnable(true))
  {
    return false;
  }
  bool result = true; /* 单步结果 */

  /* 开始写入 */

  /* 页编程指令 */
  uint8_t cmd[] = {
      SFUD_CMD_PROGRAM_256B,
      (addr >> 16) & 0xFF,
      (addr >> 8) & 0xFF,
      (addr >> 0) & 0xFF,
  };

  SpiFlash_Hal_WriteCmd(cmd, 4, (uint8_t *)data, size);

  // vTaskDelay(2);

  /* 等待flash编程完成 */
  if (!SFUD_WaitBusy(SFUD_WAIT_WRITE_LIMIT_100US))
  {
    result = false;
  }

  SFUD_WriteEnable(false);
  return result;
}

bool SFUD_ProgramData(uint32_t addr, uint16_t size, const uint8_t *data)
{
  /* 参数合法性检查 */
  if (addr > SFUD_MAX_ADDRESS || size == 0 || NULL == data)
  {
    return false;
  }
  if (size > SFUD_MAX_ADDRESS - addr) /* 写入尺寸大于地址后可读取最大容量 */
  {
    return false;
  }

  uint16_t stepSize = 0; /* 单步待写入长度 */
  bool result = true;    /* 单步结果 */

  /* 获取单步写入长度，不能超过一个页的范围 */
  if (addr + size - 1 >= ((addr & 0xFFFF00) + 0x0100)) /* 写入数据超过一个page !!!如果页大小不是256，此处需要修改!!! */
  {
    stepSize = (((addr & 0x000000FF) == 0) ? SFUD_WRITE_MAX : (SFUD_WRITE_MAX - (addr & 0x000000FF)));
  }
  else /* 输入数据在一个page内 */
  {
    stepSize = size;
  }

  /* 开始写入 */
  do
  {
    bool res = prvProgram_Single_page(addr, stepSize, data);
    if (!res)
    {
      result = false;
      break;
    }
#ifdef SFUD_VALIDATE
    /* 读取验证 */
    uint8_t readBack[SFUD_WRITE_MAX];
    SFUD_ReadData(addr, stepSize, readBack);
    for (uint16_t i = 0; i < stepSize; i++)
    {
      if (*(data + i) != *(readBack + i)) /* 数据存在错误 */
      {
        result = false;
        break;
      }
    }
#endif /* SFUD_VALIDATE */

    /* 下一步页编程指令地址计算 */
    size -= stepSize;
    data += stepSize;
    addr += stepSize;
    stepSize = (size > SFUD_WRITE_MAX) ? SFUD_WRITE_MAX : size;
    // if (size > 0)
    //   vTaskDelay(2);
  } while (size > 0);

  return result;
}

static bool SFUD_ReadStatus(t_StatusReg *status)
{
  uint8_t cmd[] = {
      SFUD_CMD_READ_STATUS,
  };

  return SpiFlash_Hal_WriteRead(cmd, 1, (uint8_t *)status, 1);
}

static bool SFUD_WriteEnable(bool enable)
{
  t_StatusReg status;
  uint8_t cmd = enable ? SFUD_CMD_SET_WRITE_ENABLE : SFUD_CMD_SET_WRITE_DISABLE;

  bool res = SpiFlash_Hal_WriteDate(&cmd, 1);

  /* 读取状态，判断写使能设置是否成功 */
  if (res && SFUD_ReadStatus(&status))
  {
    if ((enable && 1 == status.writeEnable) || (!enable && 0 == status.writeEnable))
    {
      return true;
    }
  }
  return false;
}

bool SFUD_ReadData(uint32_t addr, uint16_t size, uint8_t *data)
{
  /* 参数合法性检查 */
  if (addr > SFUD_MAX_ADDRESS || size == 0 || NULL == data)
  {
    return false;
  }
  if (size > SFUD_MAX_ADDRESS - addr) /* 读取尺寸大于地址后可读取最大容量 */
  {
    return false;
  }

  uint8_t cmd[] = {
      SFUD_CMD_READ_DATA,
      (addr >> 16) & 0xFF,
      (addr >> 8) & 0xFF,
      (addr >> 0) & 0xFF,
  };

  return SpiFlash_Hal_WriteRead(cmd, 4, data, size);
}

static bool SFUD_Erase(e_EraseType type, uint32_t addr)
{
  /* 写使能 */
  if (!SFUD_WriteEnable(true))
  {
    return false;
  }

  /* 根据擦除类型准备指令 */
  uint8_t cmd[] = {
      SFUD_CMD_ERASE_SECTOR,
      (addr >> 16) & 0xFF,
      (addr >> 8) & 0xFF,
      (addr >> 0) & 0xFF,
  };
  if (ERASE_BLOCK == type)
  {
    cmd[0] = SFUD_CMD_ERASE_BLOCK;
  }

  /* 写入指令 */
  bool res = SpiFlash_Hal_WriteDate(cmd, 4);

  /* 等待flash硬件擦除完成 */
  bool result = true;
  if (!SFUD_WaitBusy(ERASE_BLOCK == type ? SFUD_WAIT_ERASE_BLOCK_LIMIT_100US : SFUD_WAIT_ERASE_SECTOR_LIMIT_100US))
  {
    result = false;
  }

  /* 关闭写使能 */
  SFUD_WriteEnable(false);

  return result && res;
}

bool SFUD_EraseSector(uint32_t addr)
{
  if (addr > SFUD_MAX_ADDRESS)
  {
    return false;
  }
  return SFUD_Erase(ERASE_SECTOR, addr);
}

bool SFUD_EraseBlock(uint32_t addr)
{
  if (addr > SFUD_MAX_ADDRESS)
  {
    return false;
  }
  return SFUD_Erase(ERASE_BLOCK, addr);
}

#include "cmsis_compiler.h"
static bool SFUD_WaitBusy(uint32_t maxWaitCycle)
{
  t_StatusReg status;
  while (1)
  {
    if (taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    {
      for (uint16_t i = 0; i < 200; i++)
      {
        __NOP();
      }
    }
    else
      vTaskDelay(2);

    if (SFUD_ReadStatus(&status) && 0 == status.writeInProgress)
    { /* 读取到了写入完成 */
      return true;
    }
    --maxWaitCycle;
    if (0 == maxWaitCycle)
      return false;
  }
}

void SFUD_Init(void)
{
  SpiFlash_Hal_Init();
}