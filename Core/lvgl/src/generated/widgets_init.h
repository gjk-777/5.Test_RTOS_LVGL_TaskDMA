/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef WIDGET_INIT_H
#define WIDGET_INIT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "gui_guider.h"

__attribute__((unused)) void kb_event_cb(lv_event_t *e);
__attribute__((unused)) void ta_event_cb(lv_event_t *e);
#if LV_USE_ANALOGCLOCK != 0
void clock_count(int *hour, int *min, int *sec);
#endif


void screen_1_digital_clock_1_timer(lv_timer_t *timer);extern lv_obj_t * screen_2_spinbox_1;
void lv_screen_2_spinbox_1_increment_event_cb(lv_event_t * event);
void lv_screen_2_spinbox_1_decrement_event_cb(lv_event_t * event);

extern lv_calendar_date_t screen_2_calendar_1_today;
extern lv_calendar_date_t screen_2_calendar_1_highlihted_days[1];
void screen_2_calendar_1_event_handler(lv_event_t * e);
void screen_2_calendar_1_draw_part_begin_event_cb(lv_event_t * e);

#ifdef __cplusplus
}
#endif
#endif
