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
        LOG_DEBUG("[BOOTLOADER] Invalid stack pointer. Expected: 0x20000000-0x20004FFF, Got: 0x%08X\r\n", app_stack_pointer);
        return 0; // 栈指针无效
    }

    // 检查复位处理函数地址是否在APP范围内
    if ((app_reset_handler < APP_ADDR) || (app_reset_handler > FLASH_END_ADDR))
    {
        LOG_DEBUG("[BOOTLOADER] Invalid reset handler address. Expected: 0x%08X-0x%08X, Got: 0x%08X\r\n",
                  APP_ADDR, FLASH_END_ADDR, app_reset_handler);
        return 0; // 复位处理函数地址无效
    }

    // 检查复位处理函数地址是否是有效的Flash地址(通常在0x08000000之后)
    if ((app_reset_handler & 0xFF000000) != 0x08000000)
    {
        LOG_DEBUG("[BOOTLOADER] Invalid Flash address. Expected: 0x08xxxxxx, Got: 0x%08X\r\n", app_reset_handler);
        return 0; // 地址不在Flash范围内
    }

    LOG_DEBUG("[BOOTLOADER] APP is valid\r\n");
    return 1; // APP有效
}

uint8_t bootloader_get_ota_flag(void)
{
    OTA_INFO ota_info;
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));
    return ota_info.OTA_FLAG;
}
uint8_t bootloader_get_ota_index(void)
{
    OTA_INFO ota_info;
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));
    return ota_info.OTA_INDEX;
}

/**
 * @brief  清除OTA升级标志
 * @param  None
 * @retval None
 */
void bootloader_clear_ota_flag(void)
{
    OTA_INFO ota_info;

    // 读取现有的OTA信息
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    // 只清除OTA标志位，保持其他信息不变
    ota_info.OTA_FLAG = 0;

    // 擦除OTA标志所在的扇区
    W25Q128_SectorErase(OTA_FLAG_ADDRESS);

    // 写入更新后的OTA信息
    W25Q128_PageProgram(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    LOG_INFO("[BOOTLOADER] OTA flag cleared\r\n");
}

/**
 * @brief  设置OTA升级标志
 * @param  None
 * @retval None
 */
void bootloader_set_ota_flag(uint8_t ota_index)
{
    OTA_INFO ota_info;

    // 读取现有的OTA信息
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    // 只清除OTA标志位，保持其他信息不变
    ota_info.OTA_FLAG = OTA_VALID;
    ota_info.OTA_INDEX = ota_index;

    // 擦除OTA标志所在的扇区
    W25Q128_SectorErase(OTA_FLAG_ADDRESS);

    // 写入OTA标志
    W25Q128_PageProgram(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    LOG_INFO("[BOOTLOADER] OTA flag set,ota_index is %d\r\n", ota_index);
}

/**
 * @brief  设置app信息
 * @param  app_num: APP索引
 * @param  block_num: 块编号
 * @param  app_size: APP大小
 * @retval None
 */
void bootloader_set_app(uint8_t app_num, uint8_t block_num, uint16_t app_size)
{
    OTA_INFO ota_info;

    // 检查APP索引是否有效
    if (app_num >= APP_NUM)
    {
        LOG_ERROR("[BOOTLOADER] Invalid APP index: %d\r\n", app_num);
        return;
    }

    // 读取现有的OTA信息
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    // 设置指定APP的信息
    ota_info.APP[app_num].Block_NUM = block_num;
    ota_info.APP[app_num].APP_Size = app_size;
    if (app_size > 0)
        ota_info.APP[app_num].APP_Flag = APP_VALID;
    else
        ota_info.APP[app_num].APP_Flag = 0;

    // 擦除OTA标志所在的扇区
    W25Q128_SectorErase(OTA_FLAG_ADDRESS);

    // 写入更新后的OTA信息
    W25Q128_PageProgram(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    LOG_INFO("[BOOTLOADER] APP %d info set: block=%d, size=%d\r\n",
             app_num, block_num, app_size);
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
uint8_t bootloader_load_flash(uint32_t w25q_addr, uint16_t app_size)
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
 * @brief  加载指定索引的APP
 * @param  app_index : 要加载的APP索引
 * @retval None
 */
void bootloader_load_app(uint8_t app_index)
{
    // 检查APP索引是否有效
    if (app_index >= APP_NUM)
    {
        LOG_ERROR("[BOOTLOADER] Invalid APP index: %d\r\n", app_index);
        return;
    }

    LOG_INFO("[BOOTLOADER] Loading APP %d\r\n", app_index);

    OTA_INFO ota_info;
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    uint16_t app_size = ota_info.APP[app_index].APP_Size;
    uint8_t block_num = ota_info.APP[app_index].Block_NUM;

    if (ota_info.APP[app_index].APP_Flag != APP_VALID)
    {
        LOG_ERROR("[BOOTLOADER] APP %d invalid\r\n", app_index);
        return;
    }

    if (app_size > 0)
    {
        LOG_INFO("[BOOTLOADER] APP %d info: Block: %d, Size: %d bytes\r\n",
                 app_index, block_num, app_size);

        // 计算W25Q128中该APP块的起始地址
        // 每个块大小为64KB (0x10000)
        uint32_t w25q_block_addr = block_num * W25Q128_BLOCK_SIZE;

        if (!bootloader_load_flash(w25q_block_addr, app_size))
        {
            LOG_ERROR("[BOOTLOADER] Failed to load APP %d\r\n", app_index);
        }
        else
        {
            LOG_INFO("[BOOTLOADER] APP %d loaded successfully\r\n", app_index);
        }
    }
    else
    {
        LOG_INFO("[BOOTLOADER] Invalid APP %d (Size: %d, Block: %d)\r\n",
                 app_index, app_size, block_num);
        return;
    }
}

/**
 * @brief  检查是否进入BootLoader命令行模式
 * @param  timeout : 超时时间(单位: 100ms)
 * @retval 1: 进入命令行模式, 0: 不进入命令行模式
 */
uint8_t BootLoader_Enter(uint8_t timeout)
{
    LOG_INFO("[BOOTLOADER] Within %dms, enter a lowercase letter 'w' to enter BootLoader command line\r\n", timeout * 100);

    while (timeout--)
    {
        Delay_ms(100);

        // 检查是否有数据接收且第一个字符是 'w'
        if (UCB_DATA.RxDataOUT != UCB_DATA.RxDataIN &&
            UCB_DATA.RxDataOUT->start != NULL &&
            UCB_DATA.RxDataOUT->start[0] == 'w')
        {
            // 清除已处理的数据
            UCB_DATA.RxDataOUT++;
            if (UCB_DATA.RxDataOUT == UCB_DATA.RxDataEND)
            {
                UCB_DATA.RxDataOUT = &UCB_DATA.RxDataPtr[0];
            }
            return 1; // 进入命令行
        }
    }

    return 0; // 不进入命令行
}

/**
 * @brief  擦除整个APP区域
 * @param  None
 * @retval 1: 成功, 0: 失败
 */
uint8_t bootloader_erase_app_area(void)
{
    FLASH_Status status;

    // 解锁Flash
    FLASH_Unlock();

    // 擦除APP区域的所有页
    uint16_t total_pages = APP_SIZE / STM32_SECTOR_SIZE;
    uint16_t erased_pages = 0;

    LOG_DEBUG("Erasing APP area: 0x%08X - 0x%08X, Size: %d(%.2f kB) bytes, Pages: %d\r\n",
              APP_ADDR, APP_ADDR + APP_SIZE - 1, APP_SIZE, APP_SIZE / 1024.0, total_pages);

    for (uint32_t addr = APP_ADDR; addr < APP_ADDR + APP_SIZE; addr += STM32_SECTOR_SIZE)
    {
        status = FLASH_ErasePage(addr);
        if (status != FLASH_COMPLETE)
        {
            LOG_ERROR("Flash erase failed at address 0x%08X, status: %d\r\n", addr, status);
            FLASH_Lock(); // 锁定Flash
            return 0;
        }
        else
        {
            erased_pages++;
            if (total_pages > 0 && (erased_pages * 100 / total_pages) % 10 == 0)
            {
                static uint16_t last_percentage = 0;
                uint16_t percentage = erased_pages * 100 / total_pages;
                if (percentage != last_percentage)
                {
                    LOG_DEBUG("Flash erase progress: %d%% (%d/%d pages)\r\n",
                              percentage, erased_pages, total_pages);
                    last_percentage = percentage;
                }
            }
        }
    }

    // 锁定Flash
    FLASH_Lock();

    LOG_INFO("APP area erased successfully\r\n");
    return 1;
}

/**
 * @brief  显示BootLoader命令行菜单
 * @param  None
 * @retval None
 */
void BootLoaderInfo(void)
{
    uart1_printf("\r\n=======================================\r\n");
    uart1_printf("       BootLoader Command Menu\r\n");
    uart1_printf("=======================================\r\n");
    uart1_printf("[1] Erase APP area\r\n");
    uart1_printf("[2] Serial IAP download APP program\r\n");
    uart1_printf("[3] Set OTA version number\r\n");
    uart1_printf("[4] Query OTA version number\r\n");
    uart1_printf("[5] Download program to external Flash\r\n");
    uart1_printf("[6] Use program in external Flash\r\n");
    uart1_printf("[7] Reboot system\r\n");
    uart1_printf("[8] Jump to APP\r\n");
    uart1_printf("[9] Check APP validity\r\n");
    uart1_printf("=======================================\r\n");
    uart1_printf("Please enter your choice: ");
}

static uint16_t xmodem_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= (data[i] << 8);
        for (uint8_t j = 0; j < 8; j++)
        {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc;
}

uint32_t bootloader_xmodem_receive(uint32_t flash_addr)
{
    uint8_t rx_buf[133];                     // SOH + blk + ~blk + 128 + CRC
    uint8_t data_buf[XMODEM_BLOCK_SIZE * 2]; // 每两包缓存写入
    uint8_t packet_num = 1;
    uint32_t total_received = 0;
    uint32_t erased_block_base = 0xFFFFFFFF;

    uint32_t block_min = 0xFFFFFFFF;
    uint32_t block_max = 0;

    LOG_INFO("[BOOTLOADER] Starting XMODEM receive at 0x%08X\r\n", flash_addr);

    // 等待发送方
    uint32_t timeout = 10000;
    while (timeout && !UART_DATA_AVAILABLE)
    {
        timeout--;
        if (timeout % 1000 == 0)
            uart1_send_byte('C');
        Delay_ms(1);
    }

    if (timeout == 0)
    {
        LOG_ERROR("\r\n[XMODEM] No response from sender, aborting transfer\r\n");
        return 0;
    }

    while (1)
    {
        uint16_t received = 0;
        timeout = 10000000;

        while (received < 133 && timeout)
        {
            uint16_t len = uart1_read_data(&rx_buf[received], 133 - received);
            received += len;
            timeout--;
            if (len == 1 && rx_buf[0] == EOT)
                break;
        }

        if (timeout == 0)
        {
            LOG_ERROR("[BOOTLOADER] Timeout while waiting for packet\r\n");
            uart1_send_byte(CAN);
            return 0;
        }

        if (rx_buf[0] == SOH)
        {
            uint8_t pkt_num = rx_buf[1];
            uint8_t pkt_cmp = rx_buf[2];

            if ((pkt_num + pkt_cmp) != 0xFF || pkt_num != packet_num)
            {
                LOG_WARN("[BOOTLOADER] Packet number mismatch. Expected: %d, Received: %d\r\n", packet_num, pkt_num);
                uart1_send_byte(NAK);
                continue;
            }

            uint8_t *target_buf = (packet_num % 2 == 0) ? (data_buf + XMODEM_BLOCK_SIZE) : data_buf;
            memcpy(target_buf, &rx_buf[3], XMODEM_BLOCK_SIZE);

            uint16_t crc_recv = ((uint16_t)rx_buf[131] << 8) | rx_buf[132];
            uint16_t crc_calc = xmodem_crc16(target_buf, XMODEM_BLOCK_SIZE);

            if (crc_recv != crc_calc)
            {
                LOG_WARN("[BOOTLOADER] CRC error in packet %d. Recv: 0x%04X, Calc: 0x%04X\r\n", packet_num, crc_recv, crc_calc);
                uart1_send_byte(NAK);
                continue;
            }

            // 检查是否需要擦除新的 block
            uint32_t target_addr = flash_addr + total_received;
            if (target_addr >= W25Q128_SIZE_BYTES) // 检查是否超出Flash最大容量
            {
                LOG_ERROR("[XMODEM] Write address 0x%08X exceeds W25Q128 capacity (16MB), aborting\r\n", target_addr);
                uart1_send_byte(CAN);
                uart1_send_byte(CAN); // 向发送端发取消
                return 0;
            }
            uint32_t block_base = target_addr & ~(W25Q128_BLOCK_SIZE - 1); // block 起始地址
            uint32_t block_num = block_base / W25Q128_BLOCK_SIZE;

            if (block_base != erased_block_base)
            {
                LOG_INFO("[BOOTLOADER] Erasing block at 0x%08X (block %d)...\r\n", block_base, block_num);
                W25Q128_BlockErase(block_base);
                LOG_INFO("[BOOTLOADER] Block erase completed\r\n");

                erased_block_base = block_base;

                if (block_num < block_min)
                    block_min = block_num;
                if (block_num > block_max)
                    block_max = block_num;
            }

            // 每两包写一次
            if (packet_num % 2 == 0)
            {
                uint32_t write_addr = flash_addr + total_received - XMODEM_BLOCK_SIZE;
                //LOG_DEBUG("[BOOTLOADER] Writing 256B at 0x%08X\r\n", write_addr);
                W25Q128_PageProgram(write_addr, data_buf, XMODEM_BLOCK_SIZE * 2);
            }

            total_received += XMODEM_BLOCK_SIZE;
            packet_num++;
            uart1_send_byte(ACK);
        }
        else if (rx_buf[0] == EOT)
        {
            if (packet_num % 2 == 1)
            {
                uint32_t write_addr = flash_addr + total_received;
                LOG_DEBUG("[BOOTLOADER] Writing final 128B at 0x%08X\r\n", write_addr);
                W25Q128_PageProgram(write_addr, data_buf, XMODEM_BLOCK_SIZE);
                total_received += XMODEM_BLOCK_SIZE;
            }

            uart1_send_byte(ACK);

            LOG_INFO("[BOOTLOADER] Received EOT. Transmission complete\r\n");
            LOG_INFO("[BOOTLOADER] Total bytes: %d (%.2f kB)\r\n", total_received, total_received / 1024.0);

            if (block_min == block_max)
            {
                LOG_INFO("[BOOTLOADER] Flash blocks used: %d\r\n", block_min);
            }
            else
            {
                LOG_INFO("[BOOTLOADER] Flash blocks used: %d ~ %d\r\n", block_min, block_max);
            }
            LOG_INFO("[BOOTLOADER] Flash address range: 0x%06X ~ 0x%06X\r\n",
                     flash_addr, flash_addr + total_received);
            break;
        }
        else
        {
            LOG_ERROR("[BOOTLOADER] Unexpected packet start byte: 0x%02X. Aborting.\r\n", rx_buf[0]);
            uart1_send_byte(CAN);
            return 0;
        }
    }

    return total_received;
}

/**
 * @brief  Bootloader主函数
 * @param  None
 * @retval None
 */
#ifdef BUILD_BOOT_LOADER
int main(void)
{
    BOOTLOADER_STATE state = STATE_IDLE;
    W25Q128_Init();
    uart1_init(DEBUG_BAUD);

    LOG_INFO("[BOOTLOADER] Version: 1.1.0\r\n");

    if (BootLoader_Enter(20))
    {
        state = STATE_MUNU;
    }
    else
    {
        if (bootloader_get_ota_flag() == OTA_VALID)
        {
            state = STATE_OTA;
        }
        else
        {
            state = STATE_JUMP_TO_APP;
        }
    }

    while (1)
    {

        switch (state)
        {
        case STATE_MUNU:
            BootLoaderInfo();
            state = STATE_SYSTEM;
            break;
        case STATE_SYSTEM:
            // 等待用户输入
            if (UCB_DATA.RxDataOUT != UCB_DATA.RxDataIN)
            {
                uint8_t choice = UCB_DATA.RxDataOUT->start[0];
                // 清除已处理的数据
                UCB_DATA.RxDataOUT++;
                if (UCB_DATA.RxDataOUT == UCB_DATA.RxDataEND)
                {
                    UCB_DATA.RxDataOUT = &UCB_DATA.RxDataPtr[0];
                }
                switch (choice)
                {
                case '1':
                    // 擦除APP区域
                    LOG_INFO("Erasing APP area...\r\n");
                    bootloader_erase_app_area();
                    state = STATE_MUNU;
                    break;
                case '2':
                    // 串口IAP下载
                    LOG_INFO("Serial IAP download mode\r\n");
                    bootloader_set_app(0, 0x00, bootloader_xmodem_receive(0x00));
                    // bootloader_set_ota_flag(0);
                    bootloader_load_app(0);
                    bootloader_jump_to_app();
                    state = STATE_MUNU;
                    break;
                case '3':
                    // 设置OTA版本号
                    LOG_INFO("Set OTA version number\r\n");
                    // 添加设置逻辑
                    break;
                case '4':
                    // 查询OTA版本号
                    LOG_INFO("Query OTA version number\r\n");
                    // 添加查询逻辑
                    break;
                case '5':
                    // 下载到外部Flash
                    LOG_INFO("Download to external Flash\r\n");
                    // 添加下载逻辑
                    break;
                case '6':
                    // 使用外部Flash程序
                    LOG_INFO("Use program in external Flash\r\n");
                    // 添加使用逻辑
                    break;
                case '7':
                    // 重启系统
                    LOG_INFO("Rebooting system...\r\n");
                    NVIC_SystemReset();
                    break;
                case '8':
                    // 跳转到APP
                    state = STATE_JUMP_TO_APP;
                    break;
                case '9':
                    // 显示APP

                    break;
                default:
                    LOG_INFO("Invalid choice, please try again\r\n");
                    state = STATE_MUNU;
                    break;
                }
            }
            break;
        case STATE_OTA:
            LOG_INFO("[BOOTLOADER] OTA upgrade flag detected\r\n");
            bootloader_load_app(bootloader_get_ota_index());
            LOG_INFO("[BOOTLOADER] APP upgrade completed, jumping to APP\r\n");
            bootloader_jump_to_app();
            state = STATE_ERROR;
            break;
        case STATE_JUMP_TO_APP:
            LOG_INFO("[BOOTLOADER] Jumping to APP\r\n");
            bootloader_jump_to_app();
            state = STATE_ERROR;
            break;
        case STATE_ERROR:
            LOG_ERROR("[BOOTLOADER] Jumping to APP error\r\n");
            LOG_INFO("[BOOTLOADER] Enter BootLoader Command Menu\r\n");
            state = STATE_MUNU;
            break;
        case STATE_IDLE:
            break;
        default:
            break;
        }

        // if (UCB_DATA.RxDataOUT != UCB_DATA.RxDataIN)
        // {
        //     LOG_DEBUG("Received %d bytes of data this time\r\n", UCB_DATA.RxDataOUT->end - UCB_DATA.RxDataOUT->start + 1);

        //     for (int i = 0; i < UCB_DATA.RxDataOUT->end - UCB_DATA.RxDataOUT->start + 1; i++)
        //     {
        //         uart1_printf("%c", UCB_DATA.RxDataOUT->start[i]);
        //     }

        //     uart1_printf("\r\n");

        //     UCB_DATA.RxDataOUT++;

        //     if (UCB_DATA.RxDataOUT == UCB_DATA.RxDataEND)
        //     {
        //         UCB_DATA.RxDataOUT = &UCB_DATA.RxDataPtr[0];
        //     }
        // }
    }
}
#endif
