/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "CAN_handle.h"

#include "keyled.h"
#include "lcd.h"
#include "string.h"
#include "My_Touch.h"
#include "My_TaskCreate.h"

#include "touch.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_guider.h"
#include "events_init.h"
#include "timers.h"
#include "event_groups.h"
#include "lv_demo_stress.h"
#include "lv_port_disp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh7,
};
/* Definitions for IdleTimer */
osTimerId_t IdleTimerHandle;
const osTimerAttr_t IdleTimer_attributes = {
  .name = "IdleTimer"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void IdleTimerCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationTickHook(void);

/* USER CODE BEGIN 3 */
void vApplicationTickHook(void)
{
  /* This function will be called by each tick interrupt if
  configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
  added here, but the tick hook is called from an interrupt context, so
  code must not attempt to block, and only the interrupt safe FreeRTOS API
  functions can be used (those that end in FromISR()). */
  // 告诉LVGL已经过去1ms
  lv_tick_inc(1);
}
/* USER CODE END 3 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
  /* place for user code */
  HAL_SuspendTick();

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();

  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  __HAL_RCC_GPIOG_CLK_DISABLE();
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
  /* place for user code */
  HAL_ResumeTick();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_CPLT_CB_ID, LVGL_LCD_FSMC_DMA_pCallback);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of IdleTimer */
  IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, &IdleTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  User_Tasks_Init();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
lv_ui guider_ui;

extern EventGroupHandle_t StopIdle_eventhandle;
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

  for (;;)
  {
    HAL_Delay(100);
    CAN_Filter_Init(CAN_Hal_RxData_RecvCallback);
    CAN_Hal_RxTask_Init();

    lcd_init();
    tp_dev.init();         // 初始化触摸屏
    //tp_adjust();           // 校准触摸屏
   // tp_save_adjust_data(); // 保存校准数据
    lcd_clear(WHITE);      // 清除屏幕

    lv_init();                // lvgl初始化
    lv_port_disp_init();      // lvgl显示接口初始化,放在lv_init()的后面
    lv_port_indev_init();     // lvgl输入接口初始化,放在lv_init()的后面
    lv_demo_keypad_encoder(); // lvgl示例程序

    // setup_ui(&guider_ui);
    // events_init(&guider_ui);
    // lv_demo_stress();
    // MX_IWDG_Init();//启动看门狗
    //  vTaskDelay(pdMS_TO_TICKS(100));
    HAL_Delay(100);
    vTaskDelete(NULL); // 删除当前任务
  }
  /* USER CODE END StartDefaultTask */
}

/* IdleTimerCallback function */
void IdleTimerCallback(void *argument)
{
  /* USER CODE BEGIN IdleTimerCallback */
  g_ScanTimeset.IdleTimerCount += 1; // 低功耗的一个重要变量
                                     // 添加事件组有效性检查

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (g_ScanTimeset.IdleTimerCount == (g_ScanTimeset.shortTime_value * 10)) // 达到息屏时间，准备将屏幕变暗
  {
    // send the Light off message
    xEventGroupSetBitsFromISR(StopIdle_eventhandle, Idle_bit, &xHigherPriorityTaskWoken);
  }
  if (g_ScanTimeset.IdleTimerCount == (g_ScanTimeset.longTime_value * 10)) // 达到休眠时间，准备进入STOP
  {
    // send the Light off message
    g_ScanTimeset.IdleTimerCount = 0;
    xEventGroupSetBitsFromISR(StopIdle_eventhandle, Stop_bit, &xHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  /* USER CODE END IdleTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

