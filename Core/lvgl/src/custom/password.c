#include "password.h"

#include <stdio.h>
#include <string.h>
uint8_t light_high=100;
char pwstr[7];
void remove_last_char(char *str)
{
    if (str == NULL)
        return;
    size_t len = strlen(str);
    if (len > 0)
    {
        str[len - 1] = '\0'; // 将最后一个字符替换成结束符
    }
}

void input_password(uint32_t id, const char *arg)
{
    if (id == 9)
    {
        // 删除按键
        remove_last_char(pwstr);
    }
    else if (id == 11)
    {
        // 对比正确
        if (strcmp(PWcorrect, pwstr) == 0)
        {
            // 屏幕跳转
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_1, guider_ui.screen_1_del, &guider_ui.screen_del, setup_scr_screen_1, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, false, true);
        }
    }
    else
    {
       // 拼接前检查长度，限制最多6个字符
        if (strlen(pwstr) < 6)
        {
            strcat(pwstr, arg);
        }
    }

    lv_label_set_text(guider_ui.screen_Password_Lable, pwstr);
}
