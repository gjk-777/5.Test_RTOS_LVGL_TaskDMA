#ifndef __MY_TOUCH_H__
#define __MY_TOUCH_H__

#include "lcd.h"
#include "string.h"
#include "stdio.h"
//加载绘制对话框函数
void load_draw_dialog(void);

/**
 * @brief       画粗线
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       size : 线条粗细程度
 * @param       color: 线的颜色
 * @retval      无
 */
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color);

//电阻触摸屏测试函数
void rtp_test(void);

//电容触摸屏测试函数
void ctp_test(void);

void rtp_test1(void);



void touch_Sleep(void);
/**
 * @brief       触摸屏唤醒
 * @note        恢复触摸屏的正常工作状态
 * @param       无
 * @retval      无
 */
void touch_Wakeup(void);

#endif
