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
#include "spi.h"
#include "main.h"

#include "FreeRTOS.h"
#include "Semphr.h"
#include "task.h"
#include "timers.h"

// CS Pin definition (Ensure these match your hardware)
#define SFUD_CS_GPIO_PORT GPIOB
#define SFUD_CS_GPIO_PIN GPIO_PIN_14

static char log_buf[256];
static SemaphoreHandle_t sfud_mutex = NULL;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} spi_user_data_t;

static spi_user_data_t w25q128_user_data = {&hspi1, SFUD_CS_GPIO_PORT, SFUD_CS_GPIO_PIN};

void sfud_log_debug(const char *file, const long line, const char *format, ...);

static void spi_lock(const sfud_spi *spi)
{
    if (sfud_mutex != NULL)
    {
        xSemaphoreTake(sfud_mutex, portMAX_DELAY);
    }
}

static void spi_unlock(const sfud_spi *spi)
{
    if (sfud_mutex != NULL)
    {
        xSemaphoreGive(sfud_mutex);
    }
}

static void sfud_delay(void)
{
    // Simple delay if needed, or use osDelay(1)
    // volatile uint8_t i;
    // for (i = 0; i < 100; i++)
    // {
    //     __NOP(); // 执行空操作指令，延时1个时钟周期
    // }
    vTaskDelay(5); // Delay 1 tick (approx 1ms)
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    spi_user_data_t *user_data = (spi_user_data_t *)spi->user_data;

    if (write_size == 0 && read_size == 0)
        return SFUD_SUCCESS;

    // CS Low
    HAL_GPIO_WritePin(user_data->cs_port, user_data->cs_pin, GPIO_PIN_RESET);

    if (write_buf && write_size > 0)
    {
        if (HAL_SPI_Transmit(user_data->hspi, (uint8_t *)write_buf, write_size, 1000) != HAL_OK)
        {
            result = SFUD_ERR_WRITE;
        }
    }

    if (read_buf && read_size > 0)
    {
        if (HAL_SPI_Receive(user_data->hspi, read_buf, read_size, 1000) != HAL_OK)
        {
            result = SFUD_ERR_READ;
        }
    }

    // CS High
    HAL_GPIO_WritePin(user_data->cs_port, user_data->cs_pin, GPIO_PIN_SET);

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

    if (sfud_mutex == NULL)
    {
        sfud_mutex = xSemaphoreCreateMutex();
    }

    switch (flash->index)
    {
    case SFUD_W25Q128_DEVICE_INDEX:
    {
        /* Initialize CS Pin */
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* Configure GPIO pin Output Level */
        HAL_GPIO_WritePin(SFUD_CS_GPIO_PORT, SFUD_CS_GPIO_PIN, GPIO_PIN_SET);

        /* Configure GPIO pin : PB14 */
        GPIO_InitStruct.Pin = SFUD_CS_GPIO_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(SFUD_CS_GPIO_PORT, &GPIO_InitStruct);

        flash->spi.name = "SPI1";
        flash->spi.wr = spi_write_read;
        flash->spi.lock = spi_lock;
        flash->spi.unlock = spi_unlock;
        flash->spi.user_data = &w25q128_user_data;
        flash->retry.delay = sfud_delay;
        flash->retry.times = 10000;
        break;
    }
    }

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
