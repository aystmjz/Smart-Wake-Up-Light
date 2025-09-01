#include "OLED.h"

u8 Image_BW[OLED_W * OLED_H / 8];

// 初始化GPIO
uint8_t OLED_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_15);
    GPIO_SetBits(GPIOB, GPIO_Pin_3);

    OLED_RES_Clr(); // Module reset
    Delay_ms(20);   // At least 10ms delay
    OLED_RES_Set();
    Delay_ms(20); // At least 10ms delay
    return Epaper_READBUSY();
}

// 模拟SPI时序
void OLED_WR_Bus(u8 dat)
{
    u8 i;
    OLED_CS_Clr();
    for (i = 0; i < 8; i++)
    {
        OLED_SCL_Clr();
        if (dat & 0x80)
        {
            OLED_SDA_Set();
        }
        else
        {
            OLED_SDA_Clr();
        }
        OLED_SCL_Set();
        dat <<= 1;
    }
    OLED_CS_Set();
}

// 写入一个命令
void OLED_WR_REG(u8 reg)
{
    OLED_DC_Clr();
    OLED_WR_Bus(reg);
    OLED_DC_Set();
}

// 写入一个字节
void OLED_WR_DATA8(u8 dat)
{
    OLED_WR_Bus(dat);
}

PAINT Paint;

/**
 * @brief 等待E-paper显示屏忙信号结束
 * @return 0: 成功等待到空闲状态  1: 等待超时
 */
uint8_t Epaper_READBUSY(void)
{
    uint32_t timeout = 0;
    const uint32_t timeout_max = 0x8fffff;

    while (1)
    {
        if (OLED_BUSY() == 0)
        {
            return 0; // 成功等待到空闲状态
        }
        
        timeout++;
        
        if (timeout >= timeout_max)
        {
            return 1; // 等待超时
        }
    }
}

void EPD_Update(void) // 全刷
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xF7);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

void EPD_Update_Fast(void) // 快刷
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xC7);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

void EPD_Update_Partial(void) // 局刷
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xFF);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

// 初始化屏幕
void OLED_GUIInit(void)
{
    // OLED_RES_Clr(); // Module reset
    // Delay_ms(20);   // At least 10ms delay
    // OLED_RES_Set();
    // Delay_ms(20); // At least 10ms delay
    // Epaper_READBUSY();
    OLED_WR_REG(0x12); // SWRESET
    Epaper_READBUSY();

    OLED_WR_REG(0x01); // Driver output control
    OLED_WR_DATA8(0x27);
    OLED_WR_DATA8(0x01);
    OLED_WR_DATA8(0x01);

    OLED_WR_REG(0x11); // data entry mode
    OLED_WR_DATA8(0x01);

    OLED_WR_REG(0x44); // set Ram-X address start/end position
    OLED_WR_DATA8(0x00);
    OLED_WR_DATA8(0x0F); // 0x0F-->(15+1)*8=128

    OLED_WR_REG(0x45);   // set Ram-Y address start/end position
    OLED_WR_DATA8(0x27); // 0xF9-->(249+1)=250
    OLED_WR_DATA8(0x01);
    OLED_WR_DATA8(0x00);
    OLED_WR_DATA8(0x00);

    OLED_WR_REG(0x3C); // BorderWavefrom
    OLED_WR_DATA8(0x05);

    OLED_WR_REG(0x21); //  Display update control
    OLED_WR_DATA8(0x00);
    OLED_WR_DATA8(0x80);

    OLED_WR_REG(0x18); // Read built-in temperature sensor
    OLED_WR_DATA8(0x80);

    OLED_WR_REG(0x4E); // set RAM X address count to 0;
    OLED_WR_DATA8(0x00);
    OLED_WR_REG(0x4F); // set RAM Y address count to 0X199;
    OLED_WR_DATA8(0x27);
    OLED_WR_DATA8(0x01);

    Epaper_READBUSY();
}

// 全刷到显存
void OLED_Display(u8 *Image, u8 Mode)
{
    LED_ON();
    if (Mode)
    {
        OLED_GUIInit();
        unsigned int Width, Height, i, j;
        u32 k = 0;
        Width = 296;
        Height = 16;
        OLED_WR_REG(0x24);
        for (j = 0; j < Height; j++)
        {
            for (i = 0; i < Width; i++)
            {
                OLED_WR_DATA8(Image[k]);
                k++;
            }
        }
        EPD_Update();
    }
    else
    {
        EPD_Dis_PartAll(Image);
    }
    LED_OFF();
}

// 用局刷的方式刷新全屏
void EPD_Dis_PartAll(u8 *Image)
{
    unsigned int Width, Height, i, j;
    u32 k = 0;
    Width = 296;
    Height = 16;

    OLED_RES_Clr();
    Delay_ms(20);
    OLED_RES_Set();
    Delay_ms(20);

    OLED_WR_REG(0x3C);
    OLED_WR_DATA8(0x80);

    // 一定要重新设置窗口与坐标！
    OLED_WR_REG(0x44);
    OLED_WR_DATA8(0x00);
    OLED_WR_DATA8(0x0F);

    OLED_WR_REG(0x45);
    OLED_WR_DATA8(0x27);
    OLED_WR_DATA8(0x01);
    OLED_WR_DATA8(0x00);
    OLED_WR_DATA8(0x00);

    OLED_WR_REG(0x4E);
    OLED_WR_DATA8(0x00);
    OLED_WR_REG(0x4F);
    OLED_WR_DATA8(0x27);
    OLED_WR_DATA8(0x01);

    OLED_WR_REG(0x24);
    for (j = 0; j < Height; j++)
    {
        for (i = 0; i < Width; i++)
        {
            OLED_WR_DATA8(Image[k]);
            k++;
        }
    }
    EPD_Update_Partial();
}

// 清屏白色
void EPD_WhiteScreen_White(void)
{
    OLED_GUIInit();
    unsigned int i;
    OLED_WR_REG(0x24);
    for (i = 0; i < OLED_W * OLED_H; i++)
    {
        OLED_WR_DATA8(0xff);
    }
    EPD_Update();
    OLED_Clear(WHITE);
    OLED_Display(Image_BW, Part);
}

// 深度睡眠
void EPD_DeepSleep(void)
{
    OLED_WR_REG(0x10); // enter deep sleep
    OLED_WR_DATA8(0x01);
}

void EPD_WeakUp(void)
{
    OLED_RES_Clr();
    Delay_ms(20);
    OLED_RES_Set();
    Delay_ms(20);
}
void Paint_NewImage(u8 *image, u16 Width, u16 Height, u16 Rotate, u16 Color)
{
    Paint.Image = 0x00;
    Paint.Image = image;

    Paint.WidthMemory = Width;
    Paint.HeightMemory = Height;
    Paint.Color = Color;
    Paint.WidthByte = (Width % 8 == 0) ? (Width / 8) : (Width / 8 + 1);
    Paint.HeightByte = Height;
    Paint.Rotate = Rotate;
    if (Rotate == ROTATE_0 || Rotate == ROTATE_180)
    {
        Paint.Width = Width;
        Paint.Height = Height;
    }
    else
    {
        Paint.Width = Height;
        Paint.Height = Width;
    }
}

void Paint_SetPixel(u16 Xpoint, u16 Ypoint, u16 Color)
{
    u16 X, Y;
    u32 Addr;
    u8 Rdata;

    if (Xpoint >= Paint.Height || Ypoint >= Paint.Width)
    {
        return; // 超出边界，直接返回
    }

    switch (Paint.Rotate)
    {
    case 0:
        X = Paint.WidthMemory - Ypoint - 1;
        Y = Xpoint;
        break;
    case 90:
        X = Paint.WidthMemory - Xpoint - 1;
        Y = Paint.HeightMemory - Ypoint - 1;
        break;
    case 180:
        X = Ypoint;
        Y = Paint.HeightMemory - Xpoint - 1;
        break;
    case 270:
        X = Xpoint;
        Y = Ypoint;
        break;
    default:
        return;
    }
    Addr = X / 8 + Y * Paint.WidthByte;
    Rdata = Paint.Image[Addr];
    if (Color == BLACK)
    {
        Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8)); // 将对应数据位置0
    }
    else
        Paint.Image[Addr] = Rdata | (0x80 >> (X % 8)); // 将对应数据位置1
}

// 清屏函数
void OLED_Clear(u16 Color)
{
    u16 X, Y;
    u32 Addr;
    for (Y = 0; Y < Paint.HeightByte; Y++)
    {
        for (X = 0; X < Paint.WidthByte; X++)
        { // 8 pixel =  1 byte
            Addr = X + Y * Paint.WidthByte;
            Paint.Image[Addr] = Color;
        }
    }
}

// 画点函数
void OLED_DrawPoint(u16 Xpoint, u16 Ypoint, u16 Color)
{
    Paint_SetPixel(Xpoint - 1, Ypoint - 1, Color);
}

// 画直线
void OLED_DrawLine(u16 Xstart, u16 Ystart, u16 Xend, u16 Yend, u16 Color)
{
    u16 Xpoint, Ypoint;
    int dx, dy;
    int XAddway, YAddway;
    int Esp;
    u8 Dotted_Len;
    Xpoint = Xstart;
    Ypoint = Ystart;
    dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    XAddway = Xstart < Xend ? 1 : -1;
    YAddway = Ystart < Yend ? 1 : -1;

    Esp = dx + dy;
    Dotted_Len = 0;

    for (;;)
    {
        Dotted_Len++;
        OLED_DrawPoint(Xpoint, Ypoint, Color);
        if (2 * Esp >= dy)
        {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx)
        {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

// 画矩形
void OLED_DrawRectangle(u16 Xstart, u16 Ystart, u16 Xend, u16 Yend, u16 Color, u8 mode)
{
    u16 i;
    if (mode)
    {
        for (i = Ystart; i < Yend; i++)
        {
            OLED_DrawLine(Xstart, i, Xend, i, Color);
        }
    }
    else
    {
        OLED_DrawLine(Xstart, Ystart, Xend, Ystart, Color);
        OLED_DrawLine(Xstart, Ystart, Xstart, Yend, Color);
        OLED_DrawLine(Xend, Yend, Xend, Ystart, Color);
        OLED_DrawLine(Xend, Yend, Xstart, Yend, Color);
    }
}

// 画圆形
void OLED_DrawCircle(u16 X_Center, u16 Y_Center, u16 Radius, u16 Color, u8 mode)
{
    u16 Esp, sCountY;
    u16 XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    Esp = 3 - (Radius << 1);
    if (mode)
    {
        while (XCurrent <= YCurrent)
        { // Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY++)
            {
                OLED_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color); // 1
                OLED_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color); // 2
                OLED_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color); // 3
                OLED_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color); // 4
                OLED_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color); // 5
                OLED_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color); // 6
                OLED_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color); // 7
                OLED_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color);
            }
            if ((int)Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
    else
    { // Draw a hollow circle
        while (XCurrent <= YCurrent)
        {
            OLED_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color); // 1
            OLED_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color); // 2
            OLED_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color); // 3
            OLED_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color); // 4
            OLED_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color); // 5
            OLED_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color); // 6
            OLED_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color); // 7
            OLED_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color); // 0
            if ((int)Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
}

void OLED_DrawChart(u16 Xstart, u16 Ystart, u16 Width, u16 Height, u8 *Data, u8 Num, u16 Color)
{
    float x_gap, y_gap;
    if (Width > OLED_W)
        Width = OLED_W;
    if (Height > OLED_H)
        Width = OLED_H;
    x_gap = Width / (float)Num;
    y_gap = Height / (float)100;
    for (u8 i = 0; i < Num - 1; i++)
    {
        OLED_DrawLine(Xstart + i * x_gap, Ystart + Height - Data[i] * y_gap, Xstart + (i + 1) * x_gap, Ystart + Height - Data[i + 1] * y_gap, Color);
    }
}

#ifdef OLED_USE_RLE
/**
 * RLE解压缩函数，根据RLE压缩数据和索引表解码指定位置的数据
 * @param rleData: RLE压缩数据数组，存储交替的值和计数对
 * @param rleIndex: 索引表数组，记录每个字符数据块的起始位置
 * @param charIndex: 要解压的字符索引
 * @param dataIndex: 在解压后的数据中要获取的字节位置索引
 * @return: 返回解压后指定位置的字节值，如果越界则返回0
 */
uint8_t RLE_Decompress(const uint8_t *rleData, const uint16_t *rleIndex, uint8_t charIndex, uint16_t dataIndex)
{
    uint16_t start, end, pos;

    if (rleIndex[0] < (charIndex + 1))
        return 0;

    if (charIndex == 0)
    {
        start = 0;
    }
    else
    {
        start = rleIndex[charIndex];
    }
    end = rleIndex[charIndex + 1];
    pos = start;

    while (pos < end)
    {
        uint8_t value = rleData[pos++];
        uint8_t count = rleData[pos++];
        if (dataIndex < count)
            return value;
        dataIndex -= count;
    }

    // 如果越界，返回0
    return 0;
}
#endif

/**
 * @brief 显示字符
 * @param X 字符显示的起始X坐标
 * @param Y 字符显示的起始Y坐标
 * @param Char 要显示的字符
 * @param Size 字体大小
 * @param Color 显示颜色
 */
void OLED_ShowChar(u16 X, u16 Y, u8 Char, u8 Size, u16 Color)
{
    u16 i, m, temp, Size2, Char1;
    u16 x0, y0;
    X += 1, Y += 1, x0 = X, y0 = Y;
    if (Size == OLED_6X8)
        Size2 = 6;
    else
        Size2 = (Size / 8 + ((Size % 8) ? 1 : 0)) * (Size / 2); // 得到字体一个字符对应点阵集所占的字节数

    if (Size == OLED_52X104)
        Char1 = Char - '0'; // 从0开始
    else
        Char1 = Char - ' '; // 计算偏移后的值

    for (i = 0; i < Size2; i++)
    {
        if (Size == OLED_6X8)
        {
            temp = OLED_ASCII0806[Char1][i];
        } // 调用0806字体
        else if (Size == OLED_6X12)
        {
            temp = OLED_ASCII1206[Char1][i];
        } // 调用1206字体
        else if (Size == OLED_8X16)
        {
            temp = OLED_ASCII1608[Char1][i];
        } // 调用1608字体
#ifdef OLED_12X24
        else if (Size == OLED_12X24)
        {

            temp = OLED_ASCII2412[Char1][i];

        } // 调用2412字体
#endif
        else if (Size == OLED_52X104)
        {
#ifdef OLED_USE_RLE
            temp = RLE_Decompress(OLED_ASCII10452_RLE, OLED_ASCII10452_RLE_index, Char1, i);
#else
            temp = OLED_ASCII10452[Char1][i];
#endif
        }
        else
            return;
        for (m = 0; m < 8; m++)
        {
            if (temp & 0x01)
                OLED_DrawPoint(X, Y, Color);
            else
                OLED_DrawPoint(X, Y, !Color);
            temp >>= 1;
            Y++;
        }
        X++;
        if ((Size != 8) && ((X - x0) == Size / 2))
        {
            X = x0;
            y0 = y0 + 8;
        }
        Y = y0;
    }
}

// m^n
u32 OLED_Pow(u16 m, u16 n)
{
    u32 result = 1;
    while (n--)
    {
        result *= m;
    }
    return result;
}

/**
 * @brief 显示数字
 * @param X 数字显示的起始X坐标
 * @param Y 数字显示的起始Y坐标
 * @param Num 要显示的数字
 * @param Len 数字的位数
 * @param Size 字体大小
 * @param Color 显示颜色
 */
void OLED_ShowNum(u16 X, u16 Y, u32 Num, u16 Len, u8 Size, u16 Color)
{
    u8 t, temp, m = 0;
    if (Size == 8)
        m = 2;
    for (t = 0; t < Len; t++)
    {
        temp = (Num / OLED_Pow(10, Len - t - 1)) % 10;
        if (temp == 0)
        {
            OLED_ShowChar(X + (Size / 2 + m) * t, Y, '0', Size, Color);
        }
        else
        {
            OLED_ShowChar(X + (Size / 2 + m) * t, Y, temp + '0', Size, Color);
        }
    }
}

#ifdef OLED_USE_RLE
/**
 * 在OLED屏幕上显示RLE压缩格式的图像
 * @param X 图像显示的起始X坐标
 * @param Y 图像显示的起始Y坐标
 * @param Sizex 图像的宽度(像素)
 * @param Sizey 图像的高度(像素)
 * @param Image_rleData RLE压缩数据数组指针
 * @param Image_rleIndex RLE索引数组指针
 * @param ImageIndex 当前图像在RLE数据中的索引
 * @param Color 图像显示颜色
 */
void OLED_RLE_ShowImage(u16 X, u16 Y, u16 Sizex, u16 Sizey, const u8 *Image_rleData, const u16 *Image_rleIndex, u8 ImageIndex, u16 Color)
{
    u16 j = 0;
    u16 i, n, temp, m;
    u16 x0, y0;
    X += 1, Y += 1, x0 = X, y0 = Y;
    Sizey = Sizey / 8 + ((Sizey % 8) ? 1 : 0);
    for (n = 0; n < Sizey; n++)
    {
        for (i = 0; i < Sizex; i++)
        {
            temp = RLE_Decompress(Image_rleData, Image_rleIndex, ImageIndex, j);
            j++;
            for (m = 0; m < 8; m++)
            {
                if (temp & 0x01)
                    OLED_DrawPoint(X, Y, Color);
                else
                    OLED_DrawPoint(X, Y, !Color);
                temp >>= 1;
                Y++;
            }
            X++;
            if ((X - x0) == Sizex)
            {
                X = x0;
                y0 = y0 + 8;
            }
            Y = y0;
        }
    }
}
#endif

/**
 * @brief 显示图片
 * @param X 图片显示的起始X坐标
 * @param Y 图片显示的起始Y坐标
 * @param Sizex 图片宽度
 * @param Sizey 图片长度
 * @param Image 图片数组
 * @param Color 图片显示的颜色
 */
void OLED_ShowImage(u16 X, u16 Y, u16 Sizex, u16 Sizey, const u8 *Image, u16 Color)
{
    u16 j = 0;
    u16 i, n, temp, m;
    u16 x0, y0;
    X += 1, Y += 1, x0 = X, y0 = Y;
    Sizey = Sizey / 8 + ((Sizey % 8) ? 1 : 0);
    for (n = 0; n < Sizey; n++)
    {
        for (i = 0; i < Sizex; i++)
        {
            temp = Image[j];
            j++;
            for (m = 0; m < 8; m++)
            {
                if (temp & 0x01)
                    OLED_DrawPoint(X, Y, Color);
                else
                    OLED_DrawPoint(X, Y, !Color);
                temp >>= 1;
                Y++;
            }
            X++;
            if ((X - x0) == Sizex)
            {
                X = x0;
                y0 = y0 + 8;
            }
            Y = y0;
        }
    }
}

#if defined(OLED_UNICODE_8X16FONT_ADDR) || defined(OLED_UNICODE_6X12FONT_ADDR)

uint16_t utf8_to_unicode16(const uint8_t *utf8)
{
    if ((utf8[0] & 0x80) == 0x00)
    {
        // 1-byte UTF-8: 0xxxxxxx
        return utf8[0];
    }
    else if ((utf8[0] & 0xE0) == 0xC0)
    {
        // 2-byte UTF-8: 110xxxxx 10xxxxxx
        return ((utf8[0] & 0x1F) << 6) |
               (utf8[1] & 0x3F);
    }
    else if ((utf8[0] & 0xF0) == 0xE0)
    {
        // 3-byte UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
        return ((utf8[0] & 0x0F) << 12) |
               ((utf8[1] & 0x3F) << 6) |
               (utf8[2] & 0x3F);
    }
    else
    {
        // 不支持超过3字节（U+10000 以上）
        return 0xFFFD; // 返回替代字符 �
    }
}

/**
 * @brief 显示汉字单字
 * @param X 汉字显示的起始X坐标
 * @param Y 汉字显示的起始Y坐标
 * @param Hanzi 要显示的汉字，范围：字库字符
 * @param Size 汉字的字体大小
 * @param Color 显示颜色
 */
void OLED_ShowChinese(u16 X, u16 Y, u8 *Hanzi, u8 Size, u16 Color) // 汉字单字打印;
{
    u8 pIndex;
    u8 hanziData[32];
    uint32_t unicode;
    uint32_t flashAddr;

    if (Size == OLED_8X16)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi16x16[pIndex].Index, "") != 0; pIndex++)
        {
            /*找到匹配的汉字*/
            if (strcmp(OLED_Hanzi16x16[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // 跳出循环，此时pIndex的值为指定汉字的索引
            }
        }
#ifdef OLED_UNICODE_8X16FONT_ADDR
        // 没找到就从字库中寻找,字库地址从(宏)开始 16*16字模U+0000 ~ U+FFFF
        if (strcmp(OLED_Hanzi16x16[pIndex].Index, "") == 0)
        {
            // 没在内置字库中找到该汉字，尝试从Flash外置字库中读取
            // 将UTF-8编码转换为Unicode编码
            unicode = utf8_to_unicode16(Hanzi);

            // 计算在Flash中的地址: 基地址 + Unicode编码 * 每个字模的大小(32字节)
            flashAddr = OLED_UNICODE_8X16FONT_ADDR + unicode * 32;

            W25Q128_ReadData(flashAddr, hanziData, 32);
            OLED_ShowImage(X, Y, 16, 16, hanziData, Color);
        }
        else
        {
            /*将汉字字模库OLED_Hanzi16x16的指定数据以16*16的图像格式显示*/
            OLED_ShowImage(X, Y, 16, 16, OLED_Hanzi16x16[pIndex].Data, Color);
        }
#else
        OLED_ShowImage(X, Y, 16, 16, OLED_Hanzi16x16[pIndex].Data, Color);
#endif
    }
    else if (Size == OLED_6X12)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi12x12[pIndex].Index, "") != 0; pIndex++)
        {
            /*找到匹配的汉字*/
            if (strcmp(OLED_Hanzi12x12[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // 跳出循环，此时pIndex的值为指定汉字的索引
            }
        }
#ifdef OLED_UNICODE_6X12FONT_ADDR
        // 没找到就从字库中寻找,字库地址从(宏)开始 12*12字模U+0000 ~ U+FFFF，使用W25Q128_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);
        if (strcmp(OLED_Hanzi12x12[pIndex].Index, "") == 0)
        {
            // 没在内置字库中找到该汉字，尝试从Flash外置字库中读取
            // 将UTF-8编码转换为Unicode编码
            unicode = utf8_to_unicode16(Hanzi);

            // 计算在Flash中的地址: 基地址 + Unicode编码 * 每个字模的大小(24字节)
            flashAddr = OLED_UNICODE_6X12FONT_ADDR + unicode * 24;

            W25Q128_ReadData(flashAddr, hanziData, 24);
            OLED_ShowImage(X, Y, 12, 12, hanziData, Color);
        }
        else
        {
            /*将汉字字模库OLED_Hanzi12x12的指定数据以12*12的图像格式显示*/
            OLED_ShowImage(X, Y, 12, 12, OLED_Hanzi12x12[pIndex].Data, Color);
        }
#else
        OLED_ShowImage(X, Y, 12, 12, OLED_Hanzi12x12[pIndex].Data, Color);
#endif
    }
}

#else

/**
 * @brief 显示汉字单字
 * @param X 汉字显示的起始X坐标
 * @param Y 汉字显示的起始Y坐标
 * @param Hanzi 要显示的汉字，范围：字库字符
 * @param Size 汉字的字体大小
 * @param Color 显示颜色
 */
void OLED_ShowChinese(u16 X, u16 Y, u8 *Hanzi, u8 Size, u16 Color) // 汉字单字打印;
{
    u8 pIndex;
    if (Size == OLED_8X16)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi16x16[pIndex].Index, "") != 0; pIndex++)
        {
            /*找到匹配的汉字*/
            if (strcmp(OLED_Hanzi16x16[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // 跳出循环，此时pIndex的值为指定汉字的索引
            }
        }
        /*将汉字字模库OLED_Hanzi16x16的指定数据以16*16的图像格式显示*/
        OLED_ShowImage(X, Y, 16, 16, OLED_Hanzi16x16[pIndex].Data, Color);
    }
    else if (Size == OLED_6X12)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi12x12[pIndex].Index, "") != 0; pIndex++)
        {
            /*找到匹配的汉字*/
            if (strcmp(OLED_Hanzi12x12[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // 跳出循环，此时pIndex的值为指定汉字的索引
            }
        }
        /*将汉字字模库OLED_Hanzi12x12的指定数据以12*12的图像格式显示*/
        OLED_ShowImage(X, Y, 12, 12, OLED_Hanzi12x12[pIndex].Data, Color);
    }
}

#endif

/**
 * @brief 显示字符串
 * @param X 字符串显示的起始X坐标
 * @param Y 字符串显示的起始Y坐标
 * @param String 要显示的字符串，范围：字库字符组成的字符串
 * @param Size 字符串的字体大小
 * @param Color 显示颜色
 */
void OLED_ShowString(u16 X, u16 Y, u8 *String, u8 Size, u16 Color) // 中英文打印;
{
    u16 i = 0, Len = 0, height = 0, width = 0;
    height = Size;
    if (Size == OLED_6X8)
        width = 6;
    else
        width = Size / 2;

    while (String[i] != '\0') // 遍历字符串的每个字符
    {
        if (String[i] == '\n')
        {
            Y += height;
            Len = 0;
            i++;
        } // 兼容换行符
        if ((X + (Len + ((String[i] > '~') ? 2 : 1)) * width) > Paint.Height)
        {
            Y += height;
            Len = 0;
        } // 超出屏幕自动换行
        if ((Y + height) > Paint.Width)
        {
            return;
        }

        if (String[i] > '~') // 如果不属于英文字符
        {
            u8 SingleChinese[4] = {0};
            SingleChinese[0] = String[i];
            i++;
            SingleChinese[1] = String[i];
            i++;
            SingleChinese[2] = String[i];
            OLED_ShowChinese(X + Len * width, Y, SingleChinese, Size, Color);
            i++;
            Len += 2;
        }
        else /*调用OLED_ShowChar函数，依次显示每个字符*/
        {
            OLED_ShowChar(X + Len * width, Y, String[i], Size, Color);
            i++;
            Len++;
        }
    }
}

/**
 * @brief 使用printf函数打印格式化字符串
 * @param X 格式化字符串显示的起始X坐标
 * @param Y 格式化字符串显示的起始Y坐标
 * @param Width 字符串显示区域的宽度
 * @param Height 字符串显示区域的高度
 * @param Size 指定字体大小
 * @param format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
 * @param ... 格式化字符串参数列表
 */
void OLED_Printf(u16 X, u16 Y, u8 Size, u16 Color, const char *format, ...)
{
    static u8 String[OLED_PRINTF_BUF_SIZE];     // 定义字符数组
    va_list arg;                                // 定义可变参数列表数据类型的变量arg
    va_start(arg, format);                      // 从format开始，接收参数列表到arg变量
    vsprintf((char *)String, format, arg);      // 使用vsprintf打印格式化字符串和参数列表到字符数组中
    va_end(arg);                                // 结束变量arg
    OLED_ShowString(X, Y, String, Size, Color); // OLED显示字符数组（字符串）并返回字符串
}