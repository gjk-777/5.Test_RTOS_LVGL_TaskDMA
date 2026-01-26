#include "test_iap_flash.h"
#include "use_iap_flash.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "cmsis_os.h"

// Define test sector (Sector 11: 0x080E0000)
// Make sure this sector is NOT used by your application code or data!
#define TEST_FLASH_SECTOR_START 0x080E0000
#define TEST_DATA_LEN 10 // Number of 32-bit words

void Test_IAP_Flash_Run(void *argument)
{
    vTaskDelay(pdMS_TO_TICKS(50000)); // Wait 50s before running test
    uint32_t write_data[TEST_DATA_LEN];

    uint32_t read_data[TEST_DATA_LEN];
    uint32_t address = TEST_FLASH_SECTOR_START;
    uint32_t result;
    int i;
    int error_count = 0;

    printf("\r\n===================================\r\n");
    printf("       IAP Flash Test Start        \r\n");
    printf("===================================\r\n");
    printf("Test Address: 0x%08X\r\n", TEST_FLASH_SECTOR_START);

    // Initialize test data
    for (i = 0; i < TEST_DATA_LEN; i++)
    {
        write_data[i] = 0x55AA0000 + i;
    }

    // 1. Erase Sector
    printf("1. Erasing Flash Sector...\r\n");
    // Pass the address to both start and end to erase the sector containing this address
    result = FLASH_If_Erase(TEST_FLASH_SECTOR_START, TEST_FLASH_SECTOR_START);
    if (result == 0)
    {
        printf("   Erase Success.\r\n");
    }
    else
    {
        printf("   Erase Failed! Error Code: %u\r\n", result);
        return;
    }

    // 2. Verify Erase (should be all 0xFFFFFFFF)
    printf("2. Verifying Erase (Check 0xFFFFFFFF)...\r\n");
    for (i = 0; i < TEST_DATA_LEN; i++)
    {
        read_data[i] = *(__IO uint32_t *)(address + i * 4);
        printf(" Erase index %d: Read 0x%08X, Expected 0xFFFFFFFF\r\n", i, read_data[i]);
        if (read_data[i] != 0xFFFFFFFF)
        {
            printf("   Error at index %d: Read 0x%08X, Expected 0xFFFFFFFF\r\n", i, read_data[i]);
            error_count++;
        }
    }
    if (error_count == 0)
    {
        printf("   Erase Verified.\r\n");
    }
    else
    {
        printf("   Erase Verification Failed with %d errors.\r\n", error_count);
    }

    printf("3. Writing Data to Flash...\r\n");
    result = FLASH_If_Write(TEST_FLASH_SECTOR_START, write_data, TEST_DATA_LEN);
    if (result == 0)
    {
        printf("   Write Success.\r\n");
    }
    else
    {
        printf("   Write Failed! Error Code: %u\r\n", result);
        return;
    }

    // 4. Verify Write
    printf("4. Verifying Write Data...\r\n");
    error_count = 0;
    for (i = 0; i < TEST_DATA_LEN; i++)
    {
        read_data[i] = *(__IO uint32_t *)(address + i * 4);
        printf("  index %d: Read 0x%08X, Expected 0x%08X\r\n", i, read_data[i], write_data[i]);
        if (read_data[i] != write_data[i])
        {
            printf("   Error at index %d: Read 0x%08X, Expected 0x%08X\r\n", i, read_data[i], write_data[i]);
            error_count++;
        }
    }

    if (error_count == 0)
    {
        printf("   Write Verification Passed!\r\n");
    }
    else
    {
        printf("   Write Verification Failed with %d errors.\r\n", error_count);
    }

    printf("===================================\r\n");
    printf("       IAP Flash Test End          \r\n");
    printf("===================================\r\n");
    while (1)
    {
        vTaskDelay(1000);
    }
}

void Test_IAP_Flash_Init(void)
{
    xTaskCreate(Test_IAP_Flash_Run, "Test_IAP_Flash_Run", 256, NULL, 10, NULL);
}
