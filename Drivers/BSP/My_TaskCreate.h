#ifndef __MY_TASKCREATE_H__
#define __MY_TASKCREATE_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef struct Scan_power_time_t
{
    uint8_t IdleTimerCount;
    uint8_t shortTime_value;
    uint8_t longTime_value;
} Scantime_t;


extern Scantime_t g_ScanTimeset;
#define Idle_bit (1 << 0)
#define IdleBreak_bit (1 << 1)
#define Stop_bit (1 << 2)
#define StopBreak_bit (1 << 3)

void User_Tasks_Init(void);
void Lvgl_Task(void *argument);
void Stop_Task(void *argument);
void IdleEnter_Task(void *argument);

void Info_Task(void *argument);
void IWDG_Task(void *argument);
#ifdef __cplusplus
}
#endif

#endif
