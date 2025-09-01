#include "bootloader.h"

const char *DividerLine = "=======================================";

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
        LOG_ERROR("[BOOTLOADER] Invalid stack pointer. Expected: 0x20000000-0x20004FFF, Got: 0x%08X\r\n", app_stack_pointer);
        return 0; // 栈指针无效
    }

    // 检查复位处理函数地址是否在APP范围内
    if ((app_reset_handler < APP_ADDR) || (app_reset_handler > FLASH_END_ADDR))
    {
        LOG_ERROR("[BOOTLOADER] Invalid reset handler address. Expected: 0x%08X-0x%08X, Got: 0x%08X\r\n",
                  APP_ADDR, FLASH_END_ADDR, app_reset_handler);
        return 0; // 复位处理函数地址无效
    }

    // 检查复位处理函数地址是否是有效的Flash地址(通常在0x08000000之后)
    if ((app_reset_handler & 0xFF000000) != 0x08000000)
    {
        LOG_ERROR("[BOOTLOADER] Invalid Flash address. Expected: 0x08xxxxxx, Got: 0x%08X\r\n", app_reset_handler);
        return 0; // 地址不在Flash范围内
    }

    LOG_INFO("[BOOTLOADER] APP is valid\r\n");
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
    else
    {
        LOG_INFO("[BOOTLOADER] Jump to APP...\r\n");
    }

    // 复位使用的外设
    // 关闭USART1
    USART_Cmd(USART1, DISABLE);
    USART_DeInit(USART1);

    /*
    // 关闭DMA
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_DeInit(DMA1_Channel5);

    // 关闭SPI
    SPI_Cmd(SPI1, DISABLE);
    SPI_I2S_DeInit(SPI1);

    // 关闭GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, DISABLE);
    // 关闭SPI2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
    // 关闭DMA时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
    // 关闭USART时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
    */

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
    static uint16_t write_buffer[STM32_SECTOR_SIZE * 5];
    uint32_t read_addr = w25q_addr;
    uint32_t write_addr = APP_ADDR;
    uint16_t remaining_size = app_size;

    LOG_INFO("[BOOTLOADER] Loading APP from W25Q128 to Flash...\r\n");
    LOG_INFO("[BOOTLOADER] W25Q128 Addr: 0x%08X, Flash Addr: 0x%08X, Size: %d bytes (%.2f KB)\r\n",
             w25q_addr, APP_ADDR, app_size, app_size / 1024.0f);

    if ((APP_ADDR + app_size > FLASH_END_ADDR))
    {
        LOG_ERROR("[BOOTLOADER] Invalid Flash address range\r\n");
        return 0;
    }

    while (remaining_size > 0)
    {
        uint16_t read_size;
        read_size = (remaining_size > sizeof(write_buffer)) ? sizeof(write_buffer) : remaining_size;

        W25Q128_ReadData(read_addr, (uint8_t *)write_buffer, read_size);

        uint16_t write_length = (read_size + 1) / 2; // 向上取整
        stmflash_write(write_addr, write_buffer, write_length);

        LOG_INFO("[BOOTLOADER] Flashing %d bytes (%.2f KB) to 0x%08X...\r\n",
                 read_size, read_size / 1024.0f, write_addr);

        read_addr += read_size;
        write_addr += read_size;
        remaining_size -= read_size;
    }
    LOG_INFO("[BOOTLOADER] APP loading completed\r\n");
    return 1;
}

/**
 * @brief  加载指定索引的APP
 * @param  app_index : 要加载的APP索引
 * @retval 1: 成功, 0: 失败
 */
uint8_t bootloader_load_app(uint8_t app_index)
{
    // 检查APP索引是否有效
    if (app_index >= APP_NUM)
    {
        LOG_ERROR("[BOOTLOADER] Invalid APP index: %d\r\n", app_index);
        return 0;
    }

    LOG_INFO("[BOOTLOADER] Loading APP %d\r\n", app_index);

    OTA_INFO ota_info;
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    uint16_t app_size = ota_info.APP[app_index].APP_Size;
    uint8_t block_num = ota_info.APP[app_index].Block_NUM;

    if (ota_info.APP[app_index].APP_Flag != APP_VALID)
    {
        LOG_ERROR("[BOOTLOADER] APP %d invalid\r\n", app_index);
        return 0;
    }

    if (app_size > 0)
    {
        LOG_INFO("[BOOTLOADER] APP %d Block: %d, Size: %d bytes (%.2f KB)\r\n",
                 app_index, block_num, app_size, app_size / 1024.0f);

        // 计算W25Q128中该APP块的起始地址
        // 每个块大小为64KB (0x10000)
        uint32_t w25q_block_addr = block_num * W25Q128_BLOCK_SIZE;

        if (!bootloader_load_flash(w25q_block_addr, app_size))
        {
            LOG_ERROR("[BOOTLOADER] Failed to load APP %d\r\n", app_index);
            return 0;
        }
        else
        {
            LOG_INFO("[BOOTLOADER] APP %d loaded successfully\r\n", app_index);
            return 1;
        }
    }
    else
    {
        LOG_INFO("[BOOTLOADER] Invalid APP %d (Size: %d, Block: %d)\r\n",
                 app_index, app_size, block_num);
        return 0;
    }
}

/**
 * @brief  检查是否进入BootLoader命令行模式
 * @param  timeout : 超时时间(单位: 100ms)
 * @retval 1: 进入命令行模式, 0: 不进入命令行模式
 */
uint8_t BootLoader_Enter(uint8_t timeout)
{
    if (*(volatile uint32_t *)BOOT_FLAG_ADDR == BOOT_FLAG_VALUE)
    {
        *(volatile uint32_t *)BOOT_FLAG_ADDR = 0; // 清除标志
        return 1;                                 // 进入命令行
    }

    LOG_INFO("[BOOTLOADER] Within %dms, enter a lowercase letter 'w' to enter BootLoader command line\r\n", timeout * 100);

    while (timeout--)
    {
        Delay_ms(100);

        // 检查是否有数据接收且第一个字符是 'w'
        if (UART_DATA_AVAILABLE && UCB_DATA.RxDataOUT->start[0] == 'w')
        {
            uart1_update_endptr();
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

    LOG_INFO("Erasing APP area: 0x%08X - 0x%08X, Size: %d bytes , Pages: %d\r\n",
             APP_ADDR, APP_ADDR + APP_SIZE - 1, APP_SIZE, APP_SIZE / 1024.0f, total_pages);

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
                    LOG_INFO("Flash erase progress: %d%% (%d/%d pages)\r\n",
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
 * @brief  等待用户输入任意信息
 * @param  None
 * @retval None
 */
void bootloader_wait(void)
{
    uart1_printf("Press any key to continue...");

    // 等待用户输入任意字符
    while (!UART_DATA_AVAILABLE)
    {
        Delay_ms(10);
    }
    // 清除接收缓冲区中的数据
    uart1_update_endptr();

    uart1_printf("\r\n");
}

void bootloader_print_APPinfo()
{
    OTA_INFO ota_info;
    uint8_t first = 1;
    W25Q128_ReadData(OTA_FLAG_ADDRESS, (uint8_t *)&ota_info, sizeof(OTA_INFO));

    uart1_printf("\r\n%s\r\n", DividerLine);
    uart1_printf("             APP Information\r\n");
    uart1_printf("%s\r\n", DividerLine);

    // 显示各个APP的信息
    for (uint8_t i = 0; i < APP_NUM; i++)
    {
        if (ota_info.APP[i].APP_Flag == APP_VALID)
        {
            if (first)
            {
                first = 0;
            }
            else
                uart1_printf("---------------------------------------\r\n");
            uart1_printf("APP[%d]:\r\n", i);
            uart1_printf("  Block Number: %d\r\n", ota_info.APP[i].Block_NUM);
            uart1_printf("  Size: %d bytes (%.2f KB)\r\n", ota_info.APP[i].APP_Size,
                         ota_info.APP[i].APP_Size / 1024.0f);
        }
    }

    uart1_printf("%s\r\n", DividerLine);
}

/**
 * @brief  显示BootLoader命令行菜单
 * @param  None
 * @retval None
 */
void BootLoaderInfo(void)
{
    uart1_printf("\r\n%s\r\n", DividerLine);
    uart1_printf("       BootLoader Command Menu\r\n");
    uart1_printf("%s\r\n", DividerLine);
    uart1_printf("[1] Erase APP area\r\n");
    uart1_printf("[2] Serial IAP download APP program\r\n");
    uart1_printf("[3] Set OTA version number\r\n");
    uart1_printf("[4] Download file to external Flash\r\n");
    uart1_printf("[5] Download APP to external Flash\r\n");
    uart1_printf("[6] Load APP in external Flash\r\n");
    uart1_printf("[7] Reboot system\r\n");
    uart1_printf("[8] Jump to APP\r\n");
    uart1_printf("[9] Check APP info\r\n");
    uart1_printf("%s\r\n", DividerLine);
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

/**
 * @brief 通过XMODEM协议接收数据并写入Flash,自动擦除Flash块
 * @param flash_addr 接收数据写入Flash的起始地址
 * @return 返回成功接收并写入Flash的总字节数；若失败则返回0
 */
uint32_t bootloader_xmodem_receive(uint32_t flash_addr)
{
    uint8_t rx_buf[133]; // SOH + blk + ~blk + 128 + CRC
    uint8_t data_buf[XMODEM_PACKET_SIZE];
    uint8_t packet_num = 1;
    uint32_t total_received = 0;
    uint32_t write_addr = flash_addr;

    uint32_t erased_block_base = 0xFFFFFFFF;
    uint8_t block_min = 0xFF;
    uint8_t block_max = 0;

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

        while (received < 133 && --timeout)
        {
            uint16_t len = uart1_read_data(&rx_buf[received], 133 - received);
            received += len;
            if (len == 1 && rx_buf[0] == EOT)
                break;
        }

        if (timeout == 0)
        {
            uart1_send_byte(CAN);
            LOG_ERROR("[BOOTLOADER] Timeout while waiting for packet\r\n");
            return 0;
        }

        if (rx_buf[0] == SOH)
        {
            uint8_t pkt_num = rx_buf[1];
            uint8_t pkt_cmp = rx_buf[2];

            if ((pkt_num + pkt_cmp) != 0xFF || pkt_num != packet_num)
            {
                uart1_send_byte(NAK);
                LOG_WARN("[BOOTLOADER] Packet number mismatch. Expected: %d, Received: %d\r\n", packet_num, pkt_num);
                continue;
            }

            uint8_t *payload = &rx_buf[3];
            uint16_t crc_recv = ((uint16_t)rx_buf[131] << 8) | rx_buf[132];
            uint16_t crc_calc = xmodem_crc16(payload, XMODEM_PACKET_SIZE);

            if (crc_recv != crc_calc)
            {
                uart1_send_byte(NAK);
                LOG_WARN("[BOOTLOADER] CRC error in packet %d. Recv: 0x%04X, Calc: 0x%04X\r\n", packet_num, crc_recv, crc_calc);
                continue;
            }

            // 检查是否需要擦除新的 block
            uint32_t target_addr = flash_addr + total_received;
            if (target_addr >= W25Q128_FLASH_SIZE_BYTES) // 检查是否超出Flash最大容量
            {
                uart1_send_byte(CAN);
                uart1_send_byte(CAN); // 向发送端发取消
                LOG_ERROR("[XMODEM] Write address 0x%08X exceeds W25Q128 capacity (16MB), aborting\r\n", target_addr);
                return 0;
            }

            uart1_send_byte(ACK); // 提前接收
            memcpy(data_buf, payload, XMODEM_PACKET_SIZE);
            total_received += XMODEM_PACKET_SIZE;
            packet_num++;

            uint32_t block_base = target_addr & ~(W25Q128_BLOCK_SIZE - 1); // block 起始地址
            uint8_t block_num = block_base / W25Q128_BLOCK_SIZE;

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

            // LOG_INFO("[BOOTLOADER] Writing %d bytes (%.2f KB) at 0x%08X\r\n",
            //          XMODEM_PACKET_SIZE, XMODEM_PACKET_SIZE / 1024.0f, write_addr);
            W25Q128_PageProgram(write_addr, data_buf, XMODEM_PACKET_SIZE);
            write_addr += XMODEM_PACKET_SIZE;
        }
        else if (rx_buf[0] == EOT)
        {

            uart1_send_byte(ACK);

            LOG_INFO("[BOOTLOADER] Received EOT. Transmission complete\r\n");
            LOG_INFO("[BOOTLOADER] Total bytes: %d (%.2f kB)\r\n", total_received, total_received / 1024.0f);

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
            uart1_send_byte(CAN);
            LOG_ERROR("[BOOTLOADER] Unexpected packet start byte: 0x%02X. Aborting.\r\n", rx_buf[0]);
            return 0;
        }
    }

    return total_received;
}

uint32_t bootloader_xmodem_iap(void)
{
    uint8_t rx_buf[133]; // SOH + blk + ~blk + 128 + CRC
    static uint16_t flash_buffer[XMODEM_IAP_BUF_SIZE / 2];
    uint8_t packet_num = 1;
    uint32_t total_received = 0;
    uint32_t write_addr = APP_ADDR;

    uint16_t buffer_index = 0;

    LOG_INFO("[IAP] Starting XMODEM IAP to 0x%08X\r\n", write_addr);

    // 等待发送方响应
    uint32_t timeout = 10000;
    while (timeout-- && !UART_DATA_AVAILABLE)
    {
        if (timeout % 1000 == 0)
            uart1_send_byte('C');
        Delay_ms(1);
    }

    if (timeout == 0)
    {
        LOG_ERROR("[IAP] No response from sender. Aborting.\r\n");
        return 0;
    }

    while (1)
    {
        uint16_t received = 0;
        timeout = 1000000;

        while (received < 133 && --timeout)
        {
            uint16_t len = uart1_read_data(&rx_buf[received], 133 - received);
            received += len;
            if (len == 1 && rx_buf[0] == EOT)
                break;
        }

        if (timeout == 0)
        {
            uart1_send_byte(CAN);
            LOG_ERROR("[IAP] Timeout while receiving packet\r\n");
            return 0;
        }

        if (rx_buf[0] == SOH)
        {
            uint8_t pkt_num = rx_buf[1];
            uint8_t pkt_cmp = rx_buf[2];

            if ((pkt_num + pkt_cmp) != 0xFF || pkt_num != packet_num)
            {
                uart1_send_byte(NAK);
                LOG_WARN("[IAP] Packet number mismatch: expected %d, got %d\r\n", packet_num, pkt_num);
                continue;
            }

            uint8_t *payload = &rx_buf[3];
            uint16_t crc_recv = ((uint16_t)rx_buf[131] << 8) | rx_buf[132];
            uint16_t crc_calc = xmodem_crc16(payload, XMODEM_PACKET_SIZE);

            if (crc_recv != crc_calc)
            {
                uart1_send_byte(NAK);
                LOG_WARN("[IAP] CRC error in packet %d. Recv: 0x%04X, Calc: 0x%04X\r\n", packet_num, crc_recv, crc_calc);
                continue;
            }

            if ((write_addr + buffer_index + XMODEM_PACKET_SIZE) > FLASH_END_ADDR)
            {
                uart1_send_byte(CAN);
                uart1_send_byte(CAN); // 向发送端发取消
                LOG_ERROR("[IAP] Write address overflow! 0x%08X\r\n", write_addr + buffer_index);
                return 0;
            }

            uart1_send_byte(ACK); // 提前接收
            memcpy(&flash_buffer[buffer_index / 2], payload, XMODEM_PACKET_SIZE);
            buffer_index += XMODEM_PACKET_SIZE;
            total_received += XMODEM_PACKET_SIZE;
            packet_num++;

            // 缓存满就写入 Flash
            if (buffer_index >= XMODEM_IAP_BUF_SIZE)
            {
                // LOG_INFO("[IAP] Flashing %d bytes (%.2f KB) to 0x%08X...\r\n",
                //          XMODEM_IAP_BUF_SIZE, XMODEM_IAP_BUF_SIZE / 1024.0f, write_addr);
                stmflash_write(write_addr, flash_buffer, XMODEM_IAP_BUF_SIZE / 2);
                write_addr += XMODEM_IAP_BUF_SIZE;
                buffer_index -= XMODEM_IAP_BUF_SIZE;
            }
        }
        else if (rx_buf[0] == EOT)
        {
            uart1_send_byte(ACK);

            // 写入剩余不足10K的数据
            if (buffer_index > 0)
            {
                LOG_INFO("[IAP] Flashing final %d bytes (%.2f KB) to 0x%08X...\r\n", buffer_index, buffer_index / 1024.0f, write_addr);
                stmflash_write(write_addr, flash_buffer, buffer_index / 2);
            }

            LOG_INFO("[IAP] IAP complete. Total received: %d bytes (%.2f kB)\r\n", total_received, total_received / 1024.0f);
            break;
        }
        else
        {
            uart1_send_byte(CAN);
            LOG_ERROR("[IAP] Unexpected start byte: 0x%02X\r\n", rx_buf[0]);
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

    LOG_INFO("[BOOTLOADER] Version: 1.2.2\r\n");
    LOG_INFO("[BOOTLOADER] Build Date: %s %s\r\n", __DATE__, __TIME__);

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
            if (UART_DATA_AVAILABLE)
            {
                state = STATE_MUNU;
                uint8_t choice;
                uart1_read_data(&choice, sizeof(choice));
                uart1_printf("\r\n");
                switch (choice)
                {
                case '1':
                    // 擦除APP区域
                    uart1_printf("Erasing APP area...\r\n");
                    bootloader_erase_app_area();
                    break;
                case '2':
                    // 串口IAP下载
                    uart1_printf("Serial IAP download mode...\r\n");
                    uart1_printf("Warning: APP size must be less than %d bytes (%.2f KB)!\r\n",
                                 APP_SIZE, APP_SIZE / 1024.0f);
                    bootloader_wait();
                    bootloader_xmodem_iap();
                    break;
                case '3':
                    // 设置OTA版本号
                    uart1_printf("Set OTA version number...\r\n");
                    break;
                case '4':
                {
                    // 下载文件到外部Flash
                    uart1_printf("Download file to external Flash...\r\n");
                    uint32_t flash_address;
                    uart1_printf("Please enter flash address (hex): 0x"); // 请输入地址

                    while (!uart1_scanf("%lx", &flash_address))
                    {
                        Delay_ms(100);
                    }
                    uart1_printf("\r\n");

                    if (flash_address >= W25Q128_FLASH_SIZE_BYTES)
                    {
                        uart1_printf("Invalid flash address: 0x%08X\r\n", flash_address);
                        break;
                    }
                    uart1_printf("Selected flash address: 0x%08X\r\n", flash_address); // 输出确认信息

                    bootloader_wait();

                    uint32_t received_size = bootloader_xmodem_receive(flash_address);

                    if (received_size > 0)
                    {
                        uart1_printf("File downloaded to address 0x%08X successfully. Size: %d bytes (%.2f KB)\r\n",
                                     flash_address, received_size, received_size / 1024.0f);
                    }
                    else
                    {
                        uart1_printf("Failed to download file to address 0x%08X\r\n", flash_address);
                    }
                    break;
                }
                case '5':
                {
                    // 下载APP到外部Flash
                    uart1_printf("Download APP to external Flash...\r\n");
                    uint16_t app_index, block_index;
                    uart1_printf("Please select APP (0-%d): ", APP_NUM - 1);
                    while (!uart1_scanf("%hu", &app_index))
                    {
                        Delay_ms(100);
                    }
                    uart1_printf("\r\n");
                    if (app_index >= APP_NUM)
                    {
                        uart1_printf("Invalid APP index: %d\r\n", app_index);
                        break;
                    }
                    uart1_printf("Please select Block (0-%d): ", W25Q128_MAX_BLOCK_INDEX);
                    while (!uart1_scanf("%hu", &block_index))
                    {
                        Delay_ms(100);
                    }
                    uart1_printf("\r\n");
                    if (block_index > W25Q128_MAX_BLOCK_INDEX)
                    {
                        uart1_printf("Invalid Block index: %d\r\n", block_index);
                        break;
                    }
                    uart1_printf("Selected APP %d, Block %d\r\n", app_index, block_index); // 输出确认信息
                    uart1_printf("Warning: APP size must be less than %d bytes (%.2f KB)!\r\n",
                                 APP_SIZE, APP_SIZE / 1024.0f);

                    bootloader_wait();
                    bootloader_set_app(app_index, block_index, 0); // 重置APP

                    uint32_t flash_address = block_index * W25Q128_BLOCK_SIZE;
                    uint32_t received_size = bootloader_xmodem_receive(flash_address);

                    if (received_size > 0)
                    {
                        bootloader_set_app(app_index, block_index, received_size);
                        uart1_printf("APP %d downloaded to Block %d successfully. Size: %d bytes (%.2f KB)\r\n",
                                     app_index, block_index, received_size, received_size / 1024.0f);
                    }
                    else
                    {
                        uart1_printf("Failed to download APP %d to Block %d\r\n", app_index, block_index);
                    }
                    break;
                }
                case '6':
                {
                    // 使用外部Flash程序
                    uart1_printf("Load APP in external Flash...\r\n");
                    bootloader_print_APPinfo();
                    uart1_printf("Please select APP:");
                    uint16_t app_index;
                    while (!uart1_scanf("%hu", &app_index))
                    {
                        Delay_ms(100);
                    }
                    uart1_printf("\r\n");
                    if (app_index >= APP_NUM)
                    {
                        uart1_printf("Invalid APP index: %d\r\n", app_index);
                        break;
                    }
                    bootloader_load_app(app_index);
                    break;
                }
                case '7':
                    // 重启系统
                    uart1_printf("Rebooting system...\r\n");
                    NVIC_SystemReset();
                    break;
                case '8':
                    // 跳转到APP
                    uart1_printf("Jumping to APP...\r\n");
                    state = STATE_JUMP_TO_APP;
                    break;
                case '9':
                    // 显示APP信息
                    uart1_printf("Check APP info...\r\n");
                    bootloader_print_APPinfo();
                    bootloader_wait();
                    break;
                default:
                    uart1_printf("Invalid choice, please try again\r\n");
                    break;
                }
            }
            break;
        case STATE_OTA:
            LOG_INFO("[BOOTLOADER] OTA upgrade flag detected\r\n");
            if (bootloader_load_app(bootloader_get_ota_index()))
            {
                LOG_INFO("[BOOTLOADER] OTA upgrade completed\r\n");
                state = STATE_JUMP_TO_APP;
            }
            else
            {
                LOG_ERROR("[BOOTLOADER] OTA upgrade failed\r\n");
                state = STATE_ERROR;
            }
            break;
        case STATE_JUMP_TO_APP:
            bootloader_jump_to_app();
            LOG_ERROR("[BOOTLOADER] Jumping to APP error\r\n");
            state = STATE_ERROR;
            break;
        case STATE_ERROR:
            LOG_INFO("[BOOTLOADER] Enter BootLoader Command Menu\r\n");
            state = STATE_MUNU;
            break;
        case STATE_IDLE:
            break;
        }
        Delay_ms(100);
    }
}
#endif
