#include "CAN_handle.h"
#include "can.h"

#include "string.h"
#include "stdbool.h"
#include "stdint.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

typedef struct tdef_CanRxFreeRtos
{
    QueueHandle_t msg;
    TaskHandle_t task;
    EventGroupHandle_t event;
} t_CanRxFreeRtos;

#define CAN_TH_Event (1 << 0)
#define CAN_ReqiureData1_BITS (1 << 1)
#define CAN_ReqiureData2_BITS (1 << 2)

static t_CanRxFreeRtos prvtCanRxFreeRtos = {0};

#pragma pack(1)
typedef struct tdef_CanRxMsg
{
    uint32_t id;     // id
    uint8_t len;     // len
    uint8_t rtr;     // 帧类型：0表示数据帧，1表示远程帧
    uint8_t data[8]; // 数据
} t_CanRxMsg;
#pragma pack()

void CAN_Hal_RxData_RecvCallback(uint32_t id, uint8_t *data, uint8_t len, uint8_t rtr)
{
    // CAN结构
    t_CanRxMsg Rx;
    Rx.id = id;
    Rx.len = len;
    Rx.rtr = rtr;

    if (rtr == CAN_FRAME_TYPE_DATA)
    {
        memcpy(Rx.data, data, len);
    }
    else if (rtr == CAN_FRAME_TYPE_REMOTE)
    {
        // 处理遥控帧，无数据那么数据域全为0
        memset(Rx.data, 0, sizeof(Rx.data));
    }
    // 传输消息
    if (prvtCanRxFreeRtos.msg)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendToBackFromISR(prvtCanRxFreeRtos.msg, &Rx, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * @brief CAN接收任务函数
 * @details 从消息队列中接收CAN数据并进行处理
 * @param argument 任务参数（未使用）
 */
static void prvCAN_Hal_RecvTaskFunc(void *argument)
{
    t_CanRxMsg Rx;
    xEventGroupClearBits(prvtCanRxFreeRtos.event, CAN_TH_Event | CAN_ReqiureData1_BITS | CAN_ReqiureData2_BITS );
    for (;;)
    {
        /* 等待消息队列中有数据 */
        if (xQueueReceive(prvtCanRxFreeRtos.msg, &Rx, portMAX_DELAY) == pdPASS)
        {
            if (Rx.rtr == CAN_FRAME_TYPE_DATA)
            {
                printf("Receive CAN_Data Rx: ID=0x%03X, Len=%d, Data=", Rx.id, Rx.len);
                for (int i = 0; i < Rx.len; i++)
                {
                    printf("%02X ", Rx.data[i]);
                }
                printf("\n");
                /* 根据不同的CAN ID进行不同的处理 */
                switch (Rx.id)
                {
                case 0x100: /* 示例：处理ID为0x100的消息 */
                    // 比如发来温湿度数据
                    xEventGroupSetBits(prvtCanRxFreeRtos.event, CAN_TH_Event);
                    break;
                case 0x200: /* 示例：处理ID为0x200的消息 */
                    break;
                default: /* 未知ID处理 */
                    break;
                }
            }
            else if (Rx.rtr == CAN_FRAME_TYPE_REMOTE)
            {
                printf("Receive CAN_Remote Rx: ID=0x%03X,请求数据长度Len=%d\n", Rx.id, Rx.len);
                switch (Rx.id)
                {
                case 0x100:
                    xEventGroupSetBits(prvtCanRxFreeRtos.event, CAN_ReqiureData1_BITS);
                    break;
                case 0x200:
                    xEventGroupSetBits(prvtCanRxFreeRtos.event, CAN_ReqiureData2_BITS);
                    break;
                default: /* 未知ID处理 */
                    break;
                }
            }
            /* 在这里添加CAN数据处理逻辑 */
        }
    }
}
TaskHandle_t TH_Task;

/**
 * @brief CAN接收任务函数
 * @details 从消息队列中接收CAN数据并进行处理
 * @param argument 任务参数（未使用）
 */
static void TH_TaskFunc(void *argument)
{
    EventBits_t uxBits;
    for (;;)
    {
        uxBits = xEventGroupWaitBits(prvtCanRxFreeRtos.event, CAN_TH_Event | CAN_ReqiureData1_BITS | CAN_ReqiureData2_BITS , pdTRUE, pdTRUE, 100);
        if (uxBits & CAN_TH_Event)
        {
            // 处理CAN_TH_Event事件
//            printf("Temperature: %d.%d\n", Rx.data[0], Rx.data[1]);
//            printf("Humidity: %d.%d\n", Rx.data[2], Rx.data[3]);
        }
        if (uxBits & CAN_ReqiureData1_BITS)
        {
            // 处理CAN_ReqiureData1_BITS事件
        }
        if (uxBits & CAN_ReqiureData2_BITS)
        {
            // 处理CAN_ReqiureData2_BITS事件
        }
        // 这里可以添加处理CAN数据的逻辑
    }
}

void CAN_Hal_RxTask_Init()
{
    prvtCanRxFreeRtos.msg = xQueueCreate(100, sizeof(t_CanRxMsg));
    prvtCanRxFreeRtos.event = xEventGroupCreate();
    xTaskCreate(prvCAN_Hal_RecvTaskFunc, "CANRx", 512, NULL, 41, &prvtCanRxFreeRtos.task);
    xTaskCreate(TH_TaskFunc, "TH_Task", 512, NULL, 41, &TH_Task);
    // 41是osPriorityHigh1
}
