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
 * Function: It is the configure head file for this library.
 * Created on: 2016-04-23
 */

#ifndef _SFUD_CFG_H_
#define _SFUD_CFG_H_

#define SFUD_DEBUG_MODE

/* 1. 注释掉上方的自动配置宏 */
// #define SFUD_USING_SFDP
// #define SFUD_USING_FLASH_INFO_TABLE
#define SFUD_USING_QSPI

/* 2. 注释掉上方的自动设备表 */
// enum
// {
//     SFUD_W25Q64CV_DEVICE_INDEX = 0,
//     SFUD_GD25Q64B_DEVICE_INDEX = 1,
//     SFUD_W25Q128_DEVICE_INDEX = 2,
// };
//
// #define SFUD_FLASH_DEVICE_TABLE                                                  \
//     {                                                                            \
//         [SFUD_W25Q64CV_DEVICE_INDEX] = {.name = "W25Q64CV", .spi.name = "SPIX"}, \
//         [SFUD_GD25Q64B_DEVICE_INDEX] = {.name = "GD25Q64B", .spi.name = "SPIX"}, \
//         [SFUD_W25Q128_DEVICE_INDEX] = {.name = "W25Q128", .spi.name = "SPI1"},   \
//     }

/*-------如果不使用就进行下面的操作，手动添加上去---------*/

/* 3. 启用下方的配置宏 */
// #define SFUD_USING_SFDP             // 不使用SFDP
// #define SFUD_USING_FLASH_INFO_TABLE // 不使用falsh芯片信息表

/* 4. 定义你的设备索引 */
enum
{
    SFUD_W25Q128_DEVICE_INDEX = 0, // 只保留你用到的 W25Q128
};

/* 5. 手动填写 W25Q128 的详细参数 */
#define SFUD_FLASH_DEVICE_TABLE                                      \
    {                                                                \
        [SFUD_W25Q128_DEVICE_INDEX] = {                              \
            .name = "W25Q128",                                       \
            .spi.name = "SPI1",                                      \
            .chip = {                                                \
                "W25Q128", SFUD_MF_ID_WINBOND, 0x40, 0x18,           \
                16L * 1024L * 1024L, SFUD_WM_PAGE_256B, 4096, 0x20}, \
        },                                                           \
    }

#endif /* _SFUD_CFG_H_ */
