/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;

    /**
     * add your spi write and read code
     */

    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                          uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

//---------------用于实现上面的由用户实现的函数--------------------------------------------------

// #include <sfud.h>
// #include <stdarg.h>
// #include <stdio.h>
// #include "stm32f10x.h"

// static char log_buf[256]; // 打印日志和调试信息用的buf
// void sfud_log_debug(const char *file, const long line, const char *format, ...);

// /**
//  * @brief SPI用户自定义参数结构体
//  *        用于记录每个Flash设备使用的spi接口是哪个，以及cs引脚是哪个
//  */
// typedef struct
// {
//     SPI_TypeDef *spix;     // 驱动Flash芯片使用的SPI接口
//     GPIO_TypeDef *cs_gpio; // 驱动Flash芯片的CS引脚的GPIO
//     uint16_t cs_pin;       // 驱动Flash芯片的CS引脚的GPIO编号
// } spi_user_data_t;

// /**
//  * @brief 定义SPI用户自定义参数数组
//  *        用于记录每个Flash设备使用的spi接口是哪个，以及cs引脚是哪个
//  */
// static spi_user_data_t spi_user_data_table[] = {

//     [SFUD_W25Q128_DEVICE_INDEX] = {SPI1, GPIOA, GPIO_Pin_2},

// };

// /**
//  * @brief 定义延时等待函数的实现,在RTOS环境下可以使用RTOS的延时机制,这里简单使用软件延时实现
//  */
// static void sfud_delay(void)
// {
//     u32 i;
//     for (i = 0; i < 800; i++)
//         ;
// }

// /**
//  * @brief SPI接口的数据收发实现
//  * @param spi : sfud_flash对象的spi成员
//  * @param write_buf : 存储主机发送给Flash的指令的缓冲区
//  * @param write_size : write_buf中指令的长度
//  * @param read_buf : 存储Flash响应给主机的数据的缓冲区
//  * @param read_size : Flash响应给主机的数据的长度
//  * @retval 函数执行的状态
//  */
// static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf, size_t read_size)
// {

//     uint32_t i;
//     sfud_err result = SFUD_SUCCESS;
//     uint8_t send_data, read_data;
//     spi_user_data_t *spi_dev = (spi_user_data_t *)spi->user_data;

//     GPIO_ResetBits(spi_dev->cs_gpio, spi_dev->cs_pin); // CS=0，选中
//     for (i = 0; i < write_size + read_size; i++)
//     {
//         /* 先写缓冲区中的数据到 SPI 总线，数据写完后，再写 (0xFF) 到 SPI 总线 */
//         if (i < write_size)
//         {
//             send_data = *write_buf++;
//         }
//         else
//         {
//             send_data = 0xFF; // dummy write
//         }
//         /* 发送1字节数据 */
//         while (SPI_I2S_GetFlagStatus(spi_dev->spix, SPI_I2S_FLAG_TXE) == RESET)
//         { /*TODO:这里建议加超时判断，不要无限等待*/
//         }
//         SPI_I2S_SendData(spi_dev->spix, send_data);
//         /* 接收1字节数据 */
//         while (SPI_I2S_GetFlagStatus(spi_dev->spix, SPI_I2S_FLAG_RXNE) == RESET)
//         { /*TODO:这里建议加超时判断，不要无限等待*/
//         }
//         read_data = SPI_I2S_ReceiveData(spi_dev->spix);
//         /* 写缓冲区中的数据发完后，再读取 SPI 总线中的数据到读缓冲区 */
//         if (i >= write_size)
//         {
//             *read_buf++ = read_data;
//         }
//     }
//     GPIO_SetBits(spi_dev->cs_gpio, spi_dev->cs_pin); // CS=1，取消选中

//     return result;
// }

// #ifdef SFUD_USING_QSPI
// /**
//  * read flash data by QSPI
//  */
// static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
//                           uint8_t *read_buf, size_t read_size)
// {
//     sfud_err result = SFUD_SUCCESS;

//     return result;
// }
// #endif /* SFUD_USING_QSPI */

// /**
//  * @brief 实现单个指定的Flash设备上下文对象 sfud_flash 的初始化
//  * @param flash : 指定的Flash设备上下文对象
//  * @retval 函数执行的状态
//  */
// sfud_err sfud_spi_port_init(sfud_flash *flash)
// {
//     sfud_err result = SFUD_SUCCESS;

//     switch (flash->index)
//     {
//     case SFUD_W25Q128_DEVICE_INDEX:
//         // RCC、GPIO、SPI接口初始化均在BSP层中执行

//         flash->name = "Flashxx";   // Flash设备的名称，非必须，仅仅用于调试
//         flash->user_data = NULL;   // 用户自定义数据，这里不需要
//         flash->spi.name = "spixx"; // SPI接口的名称，非必须，仅仅用于调试
//         // flash->spi.qspi_read = NULL;     //这里不需要
//         flash->spi.wr = spi_write_read;                            // SPI接口的数据收发函数
//         flash->spi.lock = NULL;                                    // spi接口加锁函数，这里不需要
//         flash->spi.unlock = NULL;                                  // spi接口释放锁函数，这里不需要
//         flash->spi.user_data = &spi_user_data_table[flash->index]; // SPI接口相关的用户自定义数据
//         flash->retry.delay = sfud_delay;                           // 执行Flash操作时的延时等待函数
//         flash->retry.times = 1000;                                 // 执行flash操作时的等待尝试次数

//         break;

//     default:
//         result = SFUD_ERR_NOT_FOUND;
//     }

//     return result;
// }

// void sfud_log_debug(const char *file, const long line, const char *format, ...)
// {
//     va_list args;

//     /* args point to the first variable parameter */
//     va_start(args, format);
//     printf("[SFUD](%s:%ld) ", file, line);
//     /* must use vprintf to print */
//     vsnprintf(log_buf, sizeof(log_buf), format, args);
//     printf("%s\n", log_buf);
//     va_end(args);
// }

// void sfud_log_info(const char *format, ...)
// {
//     va_list args;

//     /* args point to the first variable parameter */
//     va_start(args, format);
//     printf("[SFUD]");
//     /* must use vprintf to print */
//     vsnprintf(log_buf, sizeof(log_buf), format, args);
//     printf("%s\n", log_buf);
//     va_end(args);
// }

//-----------------------------SFUD主要用户接口说明--------------------------
// 描述：初始化所有注册的flash对象。遍历flash_table数组，初始化所有的flash对象。
// sfud_err sfud_init(void)

//     // 描述：初始化一个指定的flash对象
//     sfud_err sfud_device_init(sfud_flash *flash);

// // 根据ID（索引）获取一个注册的flash对象
// sfud_flash *sfud_get_device(size_t index);

// // 获取flash对象数组，也就是flash_table
// const sfud_flash *sfud_get_device_table(void);

// // 描述：flash擦除操作
// // 参数flash：flash对象
// // 参数addr：擦除的起始地址
// // 参数size：擦除的空间大小，字节数
// // 注意：只要是被参数addr和size覆盖的页，不管地址是否对齐，都会被擦除
// sfud_err sfud_erase(const sfud_flash *flash, uint32_t addr, size_t size)

//     // 描述：flash编程操作
//     // 参数flash：flash对象
//     // 参数addr：写数据的起始地址
//     // 参数size：写入数据的长度，字节数
//     // 参数data：存放代写入的数据的缓冲区
//     sfud_err sfud_write(const sfud_flash *flash, uint32_t addr, size_t size, const uint8_t *data)

//     // 描述：flash读取操作
//     // 参数addr：读数据的起始地址
//     // 参数size：读取的数据的长度，字节数
//     // 参数data：存放读出的数据的缓冲区
//     sfud_err sfud_read(const sfud_flash *flash, uint32_t addr, size_t size, uint8_t *data)

//     // 描述：先擦除再写入操作，等价于sfud_erase + sfud_write
//     sfud_err sfud_erase_write(const sfud_flash *flash, uint32_t addr, size_t size, const uint8_t *data);

// // 描述：全片擦除
// sfud_err sfud_chip_erase(const sfud_flash *flash);