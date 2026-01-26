/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "password.h"
#include "keyled.h"
#include "main.h"
#include "lcd.h"

static void screen_btnm_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
    {
        lv_obj_t *obj = lv_event_get_target(e);
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        input_password(id, lv_btnmatrix_get_btn_text(obj, id));
        break;
    }
    default:
        break;
    }
}

void events_init_screen(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_btnm_1, screen_btnm_1_event_handler, LV_EVENT_ALL, ui);
}

static void screen_1_sw_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
    {
        lv_obj_t *status_obj = lv_event_get_target(e);
        int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? true : false;

        switch (status)
        {
        case (true):
        {
            lv_img_set_src(guider_ui.screen_1_img_3, &_led0_alpha_30x30);
            Buzzer_ON();

            break;
        }
        case (false):
        {
            lv_img_set_src(guider_ui.screen_1_img_3, &_ledoff_alpha_30x30);
            Buzzer_OFF();
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_1_sw_2_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
    {
        lv_obj_t *status_obj = lv_event_get_target(e);
        int status = lv_obj_has_state(status_obj, LV_STATE_CHECKED) ? true : false;

        switch (status)
        {
        case (true):
        {
            lv_img_set_src(guider_ui.screen_1_img_1, &_led1_alpha_30x30);
            LED1_ON();
            break;
        }
        case (false):
        {
            lv_img_set_src(guider_ui.screen_1_img_1, &_ledoff_alpha_30x30);
            LED1_OFF();
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_1_btn_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_PRESSED:
    {
        // ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_del, setup_scr_screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, false, true);
        memset(pwstr, 0, sizeof(pwstr));
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_2, guider_ui.screen_2_del, &guider_ui.screen_1_del, setup_scr_screen_2, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void screen_1_slider_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
    {
        char buf[8];
        lv_obj_t *slider = lv_event_get_target(e);
        light_high = lv_slider_get_value(slider); 
        lcd_set_BL(light_high);
        lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
        lv_label_set_text(guider_ui.screen_1_label_3, buf);
        lv_obj_align_to(guider_ui.screen_1_label_3, guider_ui.screen_1_slider_1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_1(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_1_sw_1, screen_1_sw_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_sw_2, screen_1_sw_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_btn_1, screen_1_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_slider_1, screen_1_slider_1_event_handler, LV_EVENT_ALL, ui);
}

static void screen_2_btn_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_PRESSED:
    {
        memset(pwstr, 0, sizeof(pwstr));
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_2_del, setup_scr_screen, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_2(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_2_btn_1, screen_2_btn_1_event_handler, LV_EVENT_ALL, ui);
}

void events_init(lv_ui *ui)
{
}
