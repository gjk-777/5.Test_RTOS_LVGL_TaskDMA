#include "My_TaskCreate.h"
#include "usart.h"
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "cmsis_os.h"
#include "lcd.h"
#include "keyled.h"
#include "touch.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demo_keypad_encoder.h"
#include "event_groups.h"
#include "timers.h"
#include "gui_guider.h"

extern osTimerId_t IdleTimerHandle;
extern uint8_t light_high;

Scantime_t g_ScanTimeset = {0, 5, 10};

osThreadId_t LvglTaskHandle;
const osThreadAttr_t LvglTask_attributes = {
    .name = "Lvgl_Task",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityHigh2,
};
osThreadId_t InfoTaskHandle;
const osThreadAttr_t InfoTask_attributes = {
    .name = "Info_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
osThreadId_t IdleEnterTaskHandle;
const osThreadAttr_t IdleEnterTask_attributes = {
    .name = "IdleEnter_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
osThreadId_t StopTaskHandle;
const osThreadAttr_t StopTask_attributes = {
    .name = "Stop_Task",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

osThreadId_t IWDGTaskHandle;
const osThreadAttr_t IWDGTask_attributes = {
    .name = "IWDG_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityHigh2,
};
EventGroupHandle_t StopIdle_eventhandle = NULL;
void User_Tasks_Init(void)
{

    osStatus_t status = osTimerStart(IdleTimerHandle, 100); // 开启RTOS定时器100ms
    if (status != osOK)
    {
        // 错误处理
        printf("timer idle start error\r\n");
    }
    StopIdle_eventhandle = xEventGroupCreate();
    if (StopIdle_eventhandle == NULL)
    {
        printf("StopIdle_eventhandle create failed!\r\n");
    }

    LvglTaskHandle = osThreadNew(Lvgl_Task, NULL, &LvglTask_attributes);
    InfoTaskHandle = osThreadNew(Info_Task, NULL, &InfoTask_attributes);
    StopTaskHandle = osThreadNew(Stop_Task, NULL, &StopTask_attributes);
    // IWDGTaskHandle = osThreadNew(IWDG_Task, NULL, &IWDGTask_attributes);
    IdleEnterTaskHandle = osThreadNew(IdleEnter_Task, NULL, &IdleEnterTask_attributes);
}

void Lvgl_Task(void *argument)
{
    EventBits_t uxBits;
    for (;;)
    {
        // 函数每1毫秒检查一次LVGL的界面不活动时间
        if (lv_disp_get_inactive_time(NULL) < 1000) // 如果1s内有用户操作，就退出低功耗
        {
            // Idle time break, set to 0
            // 唤醒任务，打破阻塞
            uxBits = xEventGroupSetBits(StopIdle_eventhandle, IdleBreak_bit);
        }

        lv_task_handler(); // 这个是lvgl的任务处理函数,处理lvgl的事件,定时器等
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void IdleEnter_Task(void *argument)
{
    EventBits_t uxBits;
    for (;;)
    {

        // 等待任一事件位被设置，最长等待50ms
        uxBits = xEventGroupWaitBits(
            StopIdle_eventhandle,     // 事件组句柄
            IdleBreak_bit | Idle_bit, // 等待的事件位
            pdTRUE,                   // 退出时清除已设置的位
            pdFALSE,                  // 不需要同时满足所有位
            pdMS_TO_TICKS(50)         // 等待超时时间
        );

        if (uxBits & IdleBreak_bit)
        {
            g_ScanTimeset.IdleTimerCount = 0;
            lcd_set_BL(light_high);
        }
        else if (uxBits & Idle_bit)
        {
            lcd_set_BL(5); // 设置亮度为5（低亮）
        }
    }
}

void Stop_Task(void *argument)
{
    EventBits_t uxBits;
    // 添加状态标志，记录系统当前是否处于低功耗模式
    static volatile uint8_t isInStopMode = 0;
    for (;;)
    {
        // 等待任一事件位被设置，最长等待500ms
        uxBits = xEventGroupWaitBits(
            StopIdle_eventhandle, // 事件组句柄
            Stop_bit,             // 等待的事件位
            pdTRUE,               // 退出时清除已设置的位
            pdFALSE,              // 不需要同时满足所有位
            pdMS_TO_TICKS(10)     // 等待超时时间
        );
        if (uxBits & Stop_bit)
        {
            // 防止重复进入STOP模式
            if (isInStopMode)
            {
                xEventGroupClearBits(StopIdle_eventhandle, Stop_bit);
                continue;
            }
            isInStopMode = 1;
            // 准备低功耗stop模式了
            lcd_clear(WHITE); // 清屏
            lcd_close_BL();
            touch_Sleep();
            g_ScanTimeset.IdleTimerCount = 0; // 重置空闲时间计数器
            HAL_UART_MspDeInit(&huart1);
            vTaskSuspendAll();                                  // 暂停所有任务
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); // 清除RTOS时钟  SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
            /* Disable SysTick Interrupt */

            // 进入STOP模式，调压器处于正常运行并等待中断唤醒wait for interrupt。
            HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

            // here is the sleep period
            // 系统暂停在此处，直到被唤醒源触发

            SystemClock_Config();

            // 重新配置系统时钟
            HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
            SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); // SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
            HAL_Delay(10);

            // 恢复UART1的中断
            HAL_UART_MspInit(&huart1);
            lcd_set_BL(light_high);
            xEventGroupClearBits(StopIdle_eventhandle, Stop_bit);

            touch_Wakeup();
            // 先恢复任务调度器

            // 给系统一些时间稳定下来
            HAL_Delay(10);

            // 强制重新初始化LVGL的内部时钟和刷新系统
            // 这会重置LVGL的时间基准，使其与系统时钟同步
            for (int i = 0; i < 5; i++)
            {
                lv_timer_handler(); // 调用LVGL的核心定时器处理函数
                HAL_Delay(5);
            }

            // 手动触发显示活动，更新LVGL内部状态
            lv_disp_trig_activity(NULL);

            // 强制整个屏幕无效，这样LVGL会重绘所有内容
            lv_obj_invalidate(lv_scr_act());

            // 强制刷新整个屏幕
            lv_refr_now(NULL);

            // 再次调用定时器处理函数，确保所有任务都被处理
            lv_timer_handler();
            HAL_Delay(10);
            lv_refr_now(NULL);
            // 14. 标记已退出低功耗模式
            isInStopMode = 0;
            xTaskResumeAll(); // 恢复任务调度
        }
    }
}

// void IWDG_Task(void *argument)
//{
//     for (;;)
//     {
//         HAL_IWDG_Refresh(&hiwdg); // 喂狗不让系统复位
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }

void Info_Task(void *argument)
{

    uint32_t i = 0;
    uint32_t TotalRunTime = 0;
    UBaseType_t task_num = 0;
    TaskStatus_t *status_array = NULL;
    TaskHandle_t task_handle = NULL;
    TaskStatus_t *task_info = NULL;
    eTaskState task_state = eInvalid;
    char *task_state_str = NULL;
    char *task_info_buf = NULL;
    /* 函数vTaskList()的使用*/
    printf("/*************第四步：函数vTaskList()的使用************/\r\n");
    task_info_buf = pvPortMalloc(500);
    vTaskList(task_info_buf); /* 获取所有任务的信息 */
    printf("任务名\t\t状态\t优先级\t剩余栈\t任务序号\r\n");
    printf("%s\r\n", task_info_buf);
    vPortFree(task_info_buf);
    printf("/********************实验结束**********************/\r\n");

    for (;;)
    {

        // Buzzer_Toggle();
        LED2_Toggle(); // 提醒程序正在进行中
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
