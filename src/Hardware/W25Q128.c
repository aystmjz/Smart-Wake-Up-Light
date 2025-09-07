#include "W25Q128.h"

/**
 * @brief W25Q128 Flash芯片初始化
 * @details 初始化W25Q128的SPI接口
 */
void W25Q128_Init(void)
{
    W25Q128_SPI_Init();
}

/**
 * @brief 读取W25Q128芯片ID
 * @param MID 制造商ID指针
 * @param DID 设备ID指针
 * @details 通过JEDEC_ID命令读取芯片的制造商ID和设备ID
 */
void W25Q128_ReadID(uint8_t *MID, uint16_t *DID)
{
    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_JEDEC_ID);
    *MID = W25Q128_SPI_SwapByte(W25Q128_DUMMY_BYTE);
    *DID = W25Q128_SPI_SwapByte(W25Q128_DUMMY_BYTE);
    *DID <<= 8;
    *DID |= W25Q128_SPI_SwapByte(W25Q128_DUMMY_BYTE);
    W25Q128_SPI_Stop();
}

/**
 * @brief 使能W25Q128写操作
 * @details 发送写使能命令，设置状态寄存器的WEL位
 */
static void W25Q128_WriteEnable(void)
{
    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_WRITE_ENABLE);
    W25Q128_SPI_Stop();
}

/**
 * @brief 等待W25Q128忙状态结束
 * @details 读取状态寄存器1，等待BUSY位清零
 */
static void W25Q128_WaitBusy(void)
{
    uint32_t Timeout;
    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_READ_STATUS_REGISTER_1);
    Timeout = 100000; // 设置超时时间
    while ((W25Q128_SPI_SwapByte(W25Q128_DUMMY_BYTE) & 0x01) == 0x01)
    {
        Timeout--;
        if (Timeout == 0)
        {
            break; // 超时退出
        }
    }
    W25Q128_SPI_Stop();
}

/**
 * @brief 页编程操作
 * @param Address 编程起始地址
 * @param DataArray 数据数组指针
 * @param Count 数据字节数
 * @details 向指定地址写入数据，必须先擦除再编程
 */
void W25Q128_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
    W25Q128_WaitBusy();
    W25Q128_WriteEnable();

    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_PAGE_PROGRAM);
    W25Q128_SPI_SwapByte(Address >> 16);
    W25Q128_SPI_SwapByte(Address >> 8);
    W25Q128_SPI_SwapByte(Address);
    for (uint16_t i = 0; i < Count; i++)
    {
        W25Q128_SPI_SwapByte(DataArray[i]);
    }
    W25Q128_SPI_Stop();
}

/**
 * @brief 扇区擦除操作(4KB)
 * @param Address 扇区地址
 * @details 擦除指定地址所在的4KB扇区
 */
void W25Q128_SectorErase(uint32_t Address)
{
    W25Q128_WaitBusy();
    W25Q128_WriteEnable();

    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_SECTOR_ERASE_4KB);
    W25Q128_SPI_SwapByte(Address >> 16);
    W25Q128_SPI_SwapByte(Address >> 8);
    W25Q128_SPI_SwapByte(Address);
    W25Q128_SPI_Stop();
}

/**
 * @brief 块擦除操作(64KB)
 * @param Address 块地址
 * @details 擦除指定地址所在的64KB块
 */
void W25Q128_BlockErase(uint32_t Address)
{
    W25Q128_WaitBusy();
    W25Q128_WriteEnable();

    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_BLOCK_ERASE_64KB);
    W25Q128_SPI_SwapByte(Address >> 16);
    W25Q128_SPI_SwapByte(Address >> 8);
    W25Q128_SPI_SwapByte(Address);
    W25Q128_SPI_Stop();
}

/**
 * @brief 读取Flash数据
 * @param Address 读取起始地址
 * @param DataArray 数据存储数组指针
 * @param Count 读取字节数
 * @details 从指定地址读取指定长度的数据
 */
void W25Q128_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
    W25Q128_SPI_Start();
    W25Q128_SPI_SwapByte(W25Q128_READ_DATA);
    W25Q128_SPI_SwapByte(Address >> 16);
    W25Q128_SPI_SwapByte(Address >> 8);
    W25Q128_SPI_SwapByte(Address);
    for (uint32_t i = 0; i < Count; i++)
    {
        DataArray[i] = W25Q128_SPI_SwapByte(W25Q128_DUMMY_BYTE);
    }
    W25Q128_SPI_Stop();
}

/**
 * @brief 读取设置参数
 * @param Set 设置参数结构体指针
 * @details 从Flash指定地址读取设置参数
 */
void W25Q128_ReadSetting(SettingTypeDef *Set)
{
    W25Q128_ReadData(SETTING_ADDRESS, (uint8_t *)Set, sizeof(*Set));
}

/**
 * @brief 写入设置参数
 * @param Set 设置参数结构体指针
 * @details 将设置参数写入Flash指定地址，先擦除扇区再编程
 */
void W25Q128_WriteSetting(SettingTypeDef *Set)
{
    W25Q128_SectorErase(SETTING_ADDRESS);
    W25Q128_PageProgram(SETTING_ADDRESS, (uint8_t *)Set, sizeof(*Set));
}
