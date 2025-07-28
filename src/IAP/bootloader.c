#include "bootloader.h"

/**
 * @brief  检查APP是否有效
 * @param  None
 * @retval 1: APP有效, 0: APP无效
 */
uint8_t bootloader_check_app_valid(void)
{
    uint32_t app_stack_pointer = *((volatile uint32_t *)APP_STACK_ADDR);
    uint32_t app_reset_handler = *((volatile uint32_t *)APP_RESET_HANDLER_ADDR);

    // 检查栈指针是否在有效SRAM范围内 0x20000000 - 0x20004FFF (20KB)
    if ((app_stack_pointer & 0xFFFF0000) != 0x20000000)
    {
        return 0; // 栈指针无效
    }

    // 检查复位处理函数地址是否在APP范围内
    if ((app_reset_handler < APP_ADDR) || (app_reset_handler > FLASH_END_ADDR))
    {
        return 0; // 复位处理函数地址无效
    }

    // 检查复位处理函数地址是否是有效的Flash地址(通常在0x08000000之后)
    if ((app_reset_handler & 0xFF000000) != 0x08000000)
    {
        return 0; // 地址不在Flash范围内
    }

    return 1; // APP有效
}

/**
 * @brief  检查是否需要进行OTA升级
 * @param  None
 * @retval 1: 需要OTA升级, 0: 不需要OTA升级
 */
uint8_t bootloader_check_ota_upgrade(void)
{
    OTA_INFO ota_info;

    // 从Flash中读取OTA标志信息
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    // 检查OTA标志是否正确
    if (ota_info.OTA_FLAG == OTA_SET_FLAG)
    {
        LOG_INFO("[BOOTLOADER] OTA upgrade flag detected\r\n");
        return 1; // 需要进行OTA升级
    }

    LOG_INFO("[BOOTLOADER] No OTA upgrade needed\r\n");
    return 0; // 不需要OTA升级
}

/**
 * @brief  清除OTA升级标志
 * @param  None
 * @retval None
 */
void bootloader_clear_ota_flag(void)
{
    OTA_INFO ota_info = {.OTA_FLAG = 0}; // 清除标志

    // 擦除OTA标志所在的扇区
    W25Q128_SectorErase(OTA_FLAG_ADDRESS);

    // 写入清除后的OTA信息
    W25Q128_PageProgram(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    LOG_INFO("[BOOTLOADER] OTA flag cleared\r\n");
}

/**
 * @brief  设置OTA升级标志
 * @param  None
 * @retval None
 */
void bootloader_set_ota_flag(void)
{
    OTA_INFO ota_info = {.OTA_FLAG = OTA_SET_FLAG};

    // 擦除OTA标志所在的扇区
    W25Q128_SectorErase(OTA_FLAG_ADDRESS);

    // 写入OTA标志
    W25Q128_PageProgram(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    LOG_INFO("[BOOTLOADER] OTA flag set\r\n");
}

/**
 * @brief  跳转到APP程序
 * @param  None
 * @retval None
 */
void bootloader_jump_to_app(void)
{
    // 检查APP是否有效
    if (!bootloader_check_app_valid())
    {
        LOG_INFO("[BOOTLOADER] Invalid APP, cannot jump\r\n");
        return;
    }

    // 获取APP的栈指针和复位处理函数
    uint32_t app_stack_pointer = *((volatile uint32_t *)APP_STACK_ADDR);
    uint32_t app_reset_handler = *((volatile uint32_t *)APP_RESET_HANDLER_ADDR);

    // 设置主栈指针
    __set_MSP(app_stack_pointer);

    // 跳转到APP复位处理函数
    void (*app_reset)(void) = (void (*)(void))app_reset_handler;
    app_reset();
}

/**
 * @brief       从W25Q128读取APP并写入内部Flash
 * @param       w25q_addr : W25Q128中APP的起始地址
 * @param       app_size  : APP大小(字节)，小于64KB
 * @retval      1: 成功, 0: 失败
 */
uint8_t bootloader_load_app(uint32_t w25q_addr, uint16_t app_size)
{
    uint8_t buffer[STM32_SECTOR_SIZE]; /* 读取缓冲区，大小为一个扇区 */
    uint32_t read_addr = w25q_addr;
    uint32_t write_addr = APP_ADDR; /* 固定使用APP_ADDR */
    uint16_t remaining_size = app_size;
    uint16_t chunk_size;

    LOG_INFO("[IAP] Loading APP from W25Q128 to Flash...\r\n");
    LOG_INFO("[IAP] W25Q128 Addr: 0x%08X, Flash Addr: 0x%08X, Size: %d bytes\r\n",
             w25q_addr, APP_ADDR, app_size);

    /* 检查地址范围是否有效 */
    if ((APP_ADDR + app_size > FLASH_END_ADDR))
    {
        LOG_ERROR("[IAP] Invalid Flash address range\r\n");
        return 0;
    }

    /* 从W25Q128读取数据并写入内部Flash */
    while (remaining_size > 0)
    {
        /* 计算本次读取的大小 */
        chunk_size = (remaining_size > sizeof(buffer)) ? sizeof(buffer) : remaining_size;

        /* 从W25Q128读取数据 */
        W25Q128_ReadData(read_addr, buffer, chunk_size);

        /* 将8位数据转换为16位数据用于Flash写入 */
        uint16_t write_buffer[STM32_SECTOR_SIZE / 2]; /* 16位数据缓冲区 */
        for (int i = 0; i < chunk_size; i += 2)
        {
            uint16_t temp = 0;
            if (i + 1 < chunk_size)
            {
                temp = ((uint16_t)buffer[i + 1] << 8) | buffer[i];
            }
            else
            {
                temp = (uint16_t)buffer[i];
            }
            write_buffer[i / 2] = temp;
        }

        /* 写入内部Flash，使用stmflash_write自动处理擦除 */
        uint16_t write_length = (chunk_size + 1) / 2; /* 向上取整计算16位数据个数 */
        stmflash_write(write_addr, write_buffer, write_length);

        /* 更新地址和剩余大小 */
        read_addr += chunk_size;
        write_addr += chunk_size;
        remaining_size -= chunk_size;

        /* 显示进度 */
        if (app_size > 0) // 防止除零错误
        {
            uint16_t progress = ((app_size - remaining_size) * 100) / app_size;
            if (progress % 10 == 0) // 每10%显示一次进度
            {
                static uint16_t last_progress = 0;
                if (progress != last_progress)
                {
                    LOG_INFO("[IAP] Progress: %d%%\r\n", progress);
                    last_progress = progress;
                }
            }
        }
    }

    LOG_INFO("[IAP] APP loading completed\r\n");
    return 1;
}

/**
 * @brief  Bootloader主函数
 * @param  None
 * @retval None
 */
void bootloader_main(void)
{
    W25Q128_Init();
    uart1_init(DEBUG_BAUD);

    if (bootloader_check_ota_upgrade())
    {
        // 进入OTA升级模式
        LOG_INFO("[BOOTLOADER] Entering OTA mode\r\n");

        // 从W25Q128读取OTA信息
        OTA_INFO ota_info;
        W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

        // 获取OTA索引
        uint8_t ota_index = ota_info.OTA_INDEX;

        // 确保OTA索引在有效范围内
        if (ota_index < APP_NUM)
        {
            uint16_t app_size = ota_info.APP[ota_index].APP_Size;
            uint8_t block_num = ota_info.APP[ota_index].Block_NUM;

            if (app_size > 0)
            {
                LOG_INFO("[BOOTLOADER] Loading APP %d, Block: %d, Size: %d bytes\r\n",
                         ota_index, block_num, app_size);

                // 计算W25Q128中该APP块的起始地址
                // 每个块大小为64KB (0x10000)
                uint32_t w25q_block_addr = block_num * 0x10000;

                if (!bootloader_load_app(w25q_block_addr, app_size))
                {
                    LOG_ERROR("[BOOTLOADER] Failed to load APP %d\r\n", ota_index);
                }
                else
                {
                    LOG_INFO("[BOOTLOADER] APP %d loaded successfully\r\n", ota_index);
                }
            }
            else
            {
                LOG_INFO("[BOOTLOADER] Invalid APP %d (Size: %d, Block: %d)\r\n",
                         ota_index, app_size, block_num);
            }
        }
        else
        {
            LOG_ERROR("[BOOTLOADER] Invalid OTA index: %d\r\n", ota_index);
        }

        // OTA升级完成后清除标志
        bootloader_clear_ota_flag();

        // 跳转到APP
        LOG_INFO("[BOOTLOADER] OTA upgrade completed, jumping to APP\r\n");
        bootloader_jump_to_app();
    }
    else
    {
        // 直接跳转到APP
        LOG_INFO("[BOOTLOADER] Jumping to APP\r\n");
        bootloader_jump_to_app();
    }

    LOG_ERROR("[BOOTLOADER] Bootloader error\r\n");

    while (1)
    {
    }
}