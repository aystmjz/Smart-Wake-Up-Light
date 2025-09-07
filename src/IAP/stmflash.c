#include "stmflash.h"

/**
 * @brief       从指定地址读取一个数据 (16位数据)
 * @param       faddr   : 读取地址 (扇区地址必须为2的倍数!!)
 * @retval      读取到的数据 (16位)
 */
uint16_t stmflash_read_halfword(uint32_t faddr)
{
    return *(volatile uint16_t *)faddr;
}

/**
 * @brief       从指定地址开始读取指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的半字(16位)个数,即2字节的倍数
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_halfword(raddr); /* 读取2个字节 */
        raddr += 2;                              /* 偏移2个字节 */
    }
}

/**
 * @brief       不检查的写入
                编程前要确保相关地址数据已经正确擦除
 * @param       waddr   : 起始地址 (扇区地址必须为2的倍数!!,不能超出范围!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 半字(16位)个数
 * @retval      无
 */
void stmflash_write_nocheck(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;
    for (i = 0; i < length; i++)
    {
        FLASH_ProgramHalfWord(waddr, pbuf[i]);
        waddr += 2; /* 指向下一个半字 */
    }
}

/**
 * @brief       从FLASH 指定位置, 写入指定长度的数据(自动擦除)
 *   @note      该函数在 STM32 内部 FLASH 指定位置写入指定长度的数据
 *              该函数会首先检查要写入的数据是否非空(全0XFFFF), 如果
 *              不是, 先擦除, 再写入, 否则直接写入.
 *              擦除时会根据需要自动恢复擦除前的数据
 * @param       waddr   : 起始地址 (扇区地址必须为2的倍数!!,不能超出范围!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 半字(16位)个数
 * @retval      无
 */
uint16_t g_flashbuf[STM32_SECTOR_SIZE / 2]; /* 缓冲区大小为2K字节 */
void stmflash_write(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint32_t secpos;    /* 扇区地址 */
    uint16_t secoff;    /* 扇区内偏移地址(16位计数) */
    uint16_t secremain; /* 扇区剩余字节数(16位计数) */
    uint16_t i;
    uint32_t offaddr; /* 去掉0X08000000后的地址 */

    if (waddr < STM32_FLASH_BASE || (waddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
    {
        return; /* 非法地址 */
    }

    FLASH_Unlock(); /* FLASH解锁 */

    offaddr = waddr - STM32_FLASH_BASE;         /* 实际偏移地址. */
    secpos  = offaddr / STM32_SECTOR_SIZE;      /* 扇区地址  0~255 for STM32F103ZET6 */
    secoff = (offaddr % STM32_SECTOR_SIZE) / 2; /* 扇区内的偏移(2个字节为计数单位.) */
    secremain = STM32_SECTOR_SIZE / 2 - secoff; /* 扇区剩余空间大小 */
    if (length <= secremain)
    {
        secremain = length; /* 不大于该扇区范围 */
    }

    while (1)
    {
        stmflash_read(secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE, g_flashbuf,
                      STM32_SECTOR_SIZE / 2); /* 读出整个扇区内容 */
        for (i = 0; i < secremain; i++)       /* 校验数据 */
        {
            if (g_flashbuf[secoff + i] != 0XFFFF)
            {
                break; /* 需要擦除 */
            }
        }
        if (i < secremain) /* 需要擦除 */
        {
            FLASH_ErasePage(secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE); /* 擦除这个扇区 */

            for (i = 0; i < secremain; i++) /* 复制 */
            {
                g_flashbuf[i + secoff] = pbuf[i];
            }
            stmflash_write_nocheck(secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE, g_flashbuf,
                                   STM32_SECTOR_SIZE / 2); /* 写入整个扇区内容 */
        }
        else
        {
            stmflash_write_nocheck(waddr, pbuf, secremain); /* 已经擦除了,直接写入扇区剩余空间. */
        }
        if (length == secremain)
        {
            break; /* 写入结束了 */
        }
        else /* 写入未结束 */
        {
            secpos++;               /* 扇区地址增1 */
            secoff = 0;             /* 偏移位置为0 */
            pbuf += secremain;      /* 指针偏移 */
            waddr += secremain * 2; /* 写地址偏移(16位数据的地址,需要*2) */
            length -= secremain;    /* 字节(16位)数目递减 */

            if (length > (STM32_SECTOR_SIZE / 2))
            {
                secremain = STM32_SECTOR_SIZE / 2; /* 下一个扇区还是写不完 */
            }
            else
            {
                secremain = length; /* 下一个扇区可以写完了 */
            }
        }
    }

    FLASH_Lock(); /* 上锁 */
}
