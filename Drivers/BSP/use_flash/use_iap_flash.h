/**
 * @file use_iap_flash.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-03-18
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __USE_IAP_FLASH_HAL_H
#define __USE_IAP_FLASH_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    uint32_t FLASH_If_Erase(uint32_t StartSector, uint32_t EndSector);
    uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t *Data, uint32_t DataLength);
    void FLASH_If_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USE_IAP_FLASH_HAL_H */

#include "main.h"
