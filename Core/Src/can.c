/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 30;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_9TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// 配置CAN的过滤器
// 一个CAN1有28个过滤器，其中14个是主过滤器，14个是从过滤器
void CAN_Filter_Config(void)
{
    CAN_FilterTypeDef can_filter;
    can_filter.SlaveStartFilterBank = 14;           // 2个CAN时,第二路CAN的开始(必须配置,共28个,这里取开始筛选器位置)
    can_filter.FilterBank = 0;                      // 过滤器0
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;  // 屏蔽模式
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT; // 32位模式
    can_filter.FilterActivation = ENABLE;
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // FIFO0
    can_filter.FilterIdHigh = 0x0000;
    can_filter.FilterIdLow = 0x0000;
    can_filter.FilterMaskIdHigh = 0x0000;
    can_filter.FilterMaskIdLow = 0x0000;
    can_filter.FilterActivation = CAN_FILTER_ENABLE; // 使能过滤器
    HAL_CAN_ConfigFilter(&hcan1, &can_filter);

    can_filter.FilterBank = 1;                      // 过滤器1
    can_filter.FilterMode = CAN_FILTERMODE_IDLIST;  // 列表模式
    can_filter.FilterScale = CAN_FILTERSCALE_16BIT; // 16位模式
    can_filter.FilterActivation = ENABLE;
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0; // FIFO0
    can_filter.FilterIdHigh = 0x0000;
    can_filter.FilterIdLow = 0x0000;
    can_filter.FilterMaskIdHigh = 0x0000;
    can_filter.FilterMaskIdLow = 0x0000;
    can_filter.FilterActivation = CAN_FILTER_ENABLE; // 使能过滤器
    HAL_CAN_ConfigFilter(&hcan1, &can_filter);
}

/*
 * @brief  以轮询方式发送CAN消息
 * @param  id: 消息ID
 * @param  TxData: 发送数据指针
 * @param  length: 发送数据长度
 */
void CAN_Tx_Message(uint32_t id, uint8_t *TxData, uint8_t length)
{
    uint32_t TxMailbox = 0; // 发送邮箱这个变量用来存储发送邮箱的编号
    uint8_t res;
    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.StdId = id;         // 标准ID
    TxHeader.ExtId = 0x00;       // 扩展ID
    TxHeader.RTR = CAN_RTR_DATA; // 数据帧
    TxHeader.IDE = CAN_ID_STD;   // 标准帧
    TxHeader.DLC = length;
    TxHeader.TransmitGlobalTime = DISABLE;            // 不使用全局时间戳
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) // 获取当前空闲发送邮箱的个数,如果为0,说明没有空闲邮箱,等待5ms
        vTaskDelay(5);
    res = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    if (res != HAL_OK)
    {
        Error_Handler();
    }
    printf("res:%d-----TxMailbox:%d\r\n", res, TxMailbox);
}

/*
 * @brief  以轮询方式接收CAN消息
 * @param  Rdata: 接收数据指针
 * @retval 接收数据长度
 */
uint8_t CAN_Rx_Message(uint8_t *Rdata)
{
    CAN_RxHeaderTypeDef can1_rx;
    uint8_t res, i;
    uint32_t id;

    res = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &can1_rx, Rdata);
    if (res == HAL_ERROR)
        return 0;
    if (can1_rx.IDE == CAN_ID_STD) // 判断是否为标准帧
    {
        printf("标准 ");
        id = can1_rx.StdId;
    }
    else
    {
        printf("扩展 ");
        id = can1_rx.ExtId;
    }

    if (can1_rx.RTR == CAN_RTR_DATA)
    {
        printf("数据帧 ID(%x)\r\n", id);
        for (i = 0; i < can1_rx.DLC; i++)
            printf("%c", Rdata[i]); // 用for循环把接收数据打印出来
        printf("\r\n");
    }
    else
    {
        printf("远程帧 ID(%x)\r\n", id);
        printf("希望请求%d字节数据\r\n", can1_rx.DLC);
    }
    return can1_rx.DLC; // 返回接收数据长度
}

typedef struct tdef_CanCallBack
{
    void (*Can_Rx_Callback)(uint32_t id, uint8_t *data, uint8_t len, uint8_t rtr);
} t_CanCallBack;
static t_CanCallBack prvtCanCallBack = {0};

/*
 * @brief  以中断方式接收CAN消息
 * @param  hcan: CAN句柄
 * @param  RxHeader: 接收头指针
 * @param  RxData: 接收数据指针
 这个中断是当CAN1的接收FIFO0有数据报文时触发
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t usData[8];
    uint8_t i = 0;
    if (hcan->Instance == CAN1)
    {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, usData) == HAL_OK)
        {
            if (prvtCanCallBack.Can_Rx_Callback != NULL)
            {
                if (RxHeader.IDE == CAN_ID_STD) // 如果是标准帧
                {
                    prvtCanCallBack.Can_Rx_Callback(RxHeader.StdId, usData, RxHeader.DLC, RxHeader.RTR);
                }
                else
                {
                    prvtCanCallBack.Can_Rx_Callback(RxHeader.ExtId, usData, RxHeader.DLC, RxHeader.RTR);
                }
            }
        }
    }
}

void CAN_Filter_Init(void (*Can_Rx_Callback)(uint32_t id, uint8_t *data, uint8_t len, uint8_t rtr))
{
    prvtCanCallBack.Can_Rx_Callback = Can_Rx_Callback;
    CAN_Filter_Config();
    // 使能CAN1的接收FIFO1中断（与回调函数匹配）
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&hcan1);
}

/* USER CODE END 1 */
