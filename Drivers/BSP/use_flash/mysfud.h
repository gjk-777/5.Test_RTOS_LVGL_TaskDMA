#include "stdint.h"
#include "stdbool.h"

#define SFUD_VALIDATE /// 预处理宏定义，设置是否开启写入flash时进行读回校验。如用littlefs操作，则可关闭关闭（littleFS自带读回校验）

/**
 * @brief 初始化
 *
 */
void SFUD_Init(void);

/**
 * @brief 读取FLASH数据
 * 通过SPI接口读取FLASH数据，长度不限（地址不能超限）
 *
 * @param addr 起始地址
 * @param size 读取长度
 * @param data 返回数据的指针
 * @return true 读取成功
 * @return false 读取失败（参数不合法导致，读取过程不会出错）
 */
bool SFUD_ReadData(uint32_t addr, uint16_t size, uint8_t *data);

/**
 * @brief 擦除扇区
 *
 * @param addr 待擦除扇区的地址
 * @return true 成功
 * @return false 失败 - 可能原因：写使能失败、擦除超时
 */
bool SFUD_EraseSector(uint32_t addr);

/**
 * @brief 擦除块
 *
 * @param addr 待擦除块的地址
 * @return true 成功
 * @return false 失败 - 可能原因：写使能失败、擦除超时
 */
bool SFUD_EraseBlock(uint32_t addr);

/**
 * @brief 将数据写入到Flash中。
 *
 * @param addr 写入起始地址
 * @param size 长度 - 长度不限制，内部按照flash页大小逐步写入
 * @param data 写入数据的指针
 * @return true 写入成功
 * @return false 写入失败 -可能原因：写使能失败、硬件超时、读取校验失败
 */
bool SFUD_ProgramData(uint32_t addr, uint16_t size, const uint8_t *data);