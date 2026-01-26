#include "My_Touch.h"
#include "Lcd.h"
#include "touch.h"
#include "stdio.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"



/**
 * @brief       触摸屏进入睡眠模式
 * @note        根据触摸屏类型进行不同的低功耗处理，配置触摸唤醒功能
 * @param       无
 * @retval      无
 */
void touch_Sleep(void)
{
        /* 电阻屏处理逻辑 */

        // 2. 配置触摸笔检测引脚为外部中断模式，用于唤醒
        GPIO_InitTypeDef gpio_init_struct;
        __HAL_RCC_GPIOB_CLK_ENABLE(); // 重新启用B端口时钟以配置中断

        gpio_init_struct.Pin = T_PEN_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_IT_FALLING; // 下降沿触发中断（触摸按下时）
        gpio_init_struct.Pull = GPIO_PULLUP;          // 上拉
        HAL_GPIO_Init(T_PEN_GPIO_PORT, &gpio_init_struct);
        // 3. 配置并启用外部中断
        HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0); // 设置优先级
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);         // 启用中断
        // 3. 禁用其他触摸屏相关GPIO时钟以降低功耗
        __HAL_RCC_GPIOC_CLK_DISABLE();          // 禁用T_CS所在端口时钟
        __HAL_RCC_GPIOF_CLK_DISABLE();          // 禁用T_MOSI所在端口时钟
        // 注意：保留GPIOB时钟，因为它包含唤醒引脚
}


/**
 * @brief       触摸屏唤醒
 * @note        恢复触摸屏的正常工作状态
 * @param       无
 * @retval      无
 */
void touch_Wakeup(void)
{

        /* 电阻屏唤醒逻辑 */
        // 1. 禁用外部中断
        HAL_NVIC_DisableIRQ(EXTI1_IRQn);

        // 2. 确保清除中断标志位
        __HAL_GPIO_EXTI_CLEAR_IT(T_PEN_GPIO_PIN);
        // 2. 重新启用所有触摸屏相关GPIO时钟
        T_PEN_GPIO_CLK_ENABLE();
        T_CS_GPIO_CLK_ENABLE();
        T_MISO_GPIO_CLK_ENABLE();
        T_MOSI_GPIO_CLK_ENABLE();
        T_CLK_GPIO_CLK_ENABLE();
        //4. 添加短暂延时，确保时钟稳定
        HAL_Delay(5);
        // 2. 重新初始化触摸屏
        tp_init();
    
}
/**
 * @brief       外部中断1处理函数
 * @note        用于电阻屏触摸唤醒
 * @param       无
 * @retval      无
 */
void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(T_PEN_GPIO_PIN) != RESET)
    {
        // 触摸事件发生，唤醒系统
        //touch_Wakeup();
        // 清除中断标志位
        __HAL_GPIO_EXTI_CLEAR_IT(T_PEN_GPIO_PIN);
    }
}
void load_draw_dialog(void)
{
    lcd_clear(WHITE);                                                /* 清屏 */
    lcd_show_string(lcddev.width - 24, 0, 200, 16, 16, "RST", BLUE); /* 显示清屏区域 */
}

/**
 * @brief       画粗线
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       size : 线条粗细程度
 * @param       color: 线的颜色
 * @retval      无
 */
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;

    if (x1 < size || x2 < size || y1 < size || y2 < size)
        return;

    delta_x = x2 - x1; /* 计算坐标增量 */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* 设置单步方向 */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* 垂直线 */
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0; /* 水平线 */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x; /* 选取基本增量坐标轴 */
    else
        distance = delta_y;

    for (t = 0; t <= distance + 1; t++) /* 画线输出 */
    {
        lcd_fill_circle(row, col, size, color); /* 画点 */
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       电阻触摸屏测试函数
 * @param       无
 * @retval      无
 */
void rtp_test(void)
{
    uint8_t i = 0;
    while (1)
    {
        tp_dev.scan(0);

        if (tp_dev.sta & TP_PRES_DOWN)  /* 触摸屏被按下 */
        {
            if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
            {
                if (tp_dev.x[0] > (lcddev.width - 24) && tp_dev.y[0] < 16)
                {
                    load_draw_dialog(); /* 清除 */
                }
                else
                {
                    tp_draw_big_point(tp_dev.x[0], tp_dev.y[0], RED);   /* 画点 */
                }
            }
        }
      
        else 
        {
            vTaskDelay(10);       /* 没有按键按下的时候 */
        }


    }
}
void rtp_test1(void)
{
    uint8_t i = 0;

        tp_dev.scan(0);

        if (tp_dev.sta & TP_PRES_DOWN)  /* 触摸屏被按下 */
        {
            if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
            {
                if (tp_dev.x[0] > (lcddev.width - 24) && tp_dev.y[0] < 16)
                {
                    load_draw_dialog(); /* 清除 */
                }
                else
                {
                    tp_draw_big_point(tp_dev.x[0], tp_dev.y[0], RED);   /* 画点 */
                }
            }
        }
      
        else 
        {
            vTaskDelay(10);       /* 没有按键按下的时候 */
        }


    
}
/* 10个触控点的颜色(电容触摸屏用) */
const uint16_t POINT_COLOR_TBL[10] = {RED, GREEN, BLUE, BROWN, YELLOW, MAGENTA, CYAN, LIGHTBLUE, BRRED, GRAY};

/**
 * @brief       电容触摸屏测试函数
 * @param       无
 * @retval      无
 */
void ctp_test(void)
{
    uint8_t t = 0;
    uint8_t i = 0;
    uint16_t lastpos[10][2];        /* 最后一次的数据 */
    uint8_t maxp = 5;

    if (lcddev.id == 0x1018)maxp = 10;

    while (1)
    {
        tp_dev.scan(0);

        for (t = 0; t < maxp; t++)
        {
            if ((tp_dev.sta) & (1 << t))
            {
                if (tp_dev.x[t] < lcddev.width && tp_dev.y[t] < lcddev.height)  /* 坐标在屏幕范围内 */
                {
                    if (lastpos[t][0] == 0xFFFF)
                    {
                        lastpos[t][0] = tp_dev.x[t];
                        lastpos[t][1] = tp_dev.y[t];
                    }

                    lcd_draw_bline(lastpos[t][0], lastpos[t][1], tp_dev.x[t], tp_dev.y[t], 2, POINT_COLOR_TBL[t]); /* 画线 */
                    lastpos[t][0] = tp_dev.x[t];
                    lastpos[t][1] = tp_dev.y[t];

                    if (tp_dev.x[t] > (lcddev.width - 24) && tp_dev.y[t] < 20)
                    {
                        load_draw_dialog();/* 清除 */
                    }
                }
            }
            else 
            {
                lastpos[t][0] = 0xFFFF;
            }
        }

        HAL_Delay(5);
        i++;

    }
}

