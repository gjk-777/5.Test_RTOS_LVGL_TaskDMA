#ifndef __CAN_HANDLE_H__
#define __CAN_HANDLE_H__

#include <stdint.h>
#include <stdbool.h>

// CAN帧类型定义
#define CAN_FRAME_TYPE_DATA 0   // 数据帧
#define CAN_FRAME_TYPE_REMOTE 2 // 远程帧



void CAN_Hal_RxData_RecvCallback(uint32_t id, uint8_t *data, uint8_t len, uint8_t rtr);

void CAN_Hal_RxTask_Init();

#endif
