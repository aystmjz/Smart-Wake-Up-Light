#include "OLED.h"

u8 Image_BW[OLED_W * OLED_H / 8];

// ��ʼ��GPIO
void OLED_Init(void)
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
    Epaper_READBUSY();
}

// ģ��SPIʱ��
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

// д��һ������
void OLED_WR_REG(u8 reg)
{
    OLED_DC_Clr();
    OLED_WR_Bus(reg);
    OLED_DC_Set();
}

// д��һ���ֽ�
void OLED_WR_DATA8(u8 dat)
{
    OLED_WR_Bus(dat);
}

PAINT Paint;

void Epaper_READBUSY()
{
    while (1)
    {
        if (OLED_BUSY() == 0)
        {
            break;
        }
    }
}

void EPD_Update(void) // ȫˢ
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xF7);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

void EPD_Update_Fast(void) // ��ˢ
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xC7);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

void EPD_Update_Partial(void) // ��ˢ
{
    OLED_WR_REG(0x22);
    OLED_WR_DATA8(0xFF);
    OLED_WR_REG(0x20);
    Epaper_READBUSY();
}

// ��ʼ����Ļ
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

// ȫˢ���Դ�
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

// �þ�ˢ�ķ�ʽˢ��ȫ��
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

    // һ��Ҫ�������ô��������꣡
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

// ������ɫ
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

// ���˯��
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
        Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8)); // ����Ӧ����λ��0
    }
    else
        Paint.Image[Addr] = Rdata | (0x80 >> (X % 8)); // ����Ӧ����λ��1
}

// ��������
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

// ���㺯��
void OLED_DrawPoint(u16 Xpoint, u16 Ypoint, u16 Color)
{
    Paint_SetPixel(Xpoint - 1, Ypoint - 1, Color);
}

// ��ֱ��
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

// ������
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

// ��Բ��
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

/**
 * @brief ��ʾ�ַ�
 * @param X �ַ���ʾ����ʼX����
 * @param Y �ַ���ʾ����ʼY����
 * @param Char Ҫ��ʾ���ַ�
 * @param Size �����С
 * @param Color ��ʾ��ɫ
 */
void OLED_ShowChar(u16 X, u16 Y, u8 Char, u8 Size, u16 Color)
{
    u16 i, m, temp, Size2, Char1;
    u16 x0, y0;
    X += 1, Y += 1, x0 = X, y0 = Y;
    if (Size == 8)
        Size2 = 6;
    else
        Size2 = (Size / 8 + ((Size % 8) ? 1 : 0)) * (Size / 2); // �õ�����һ���ַ���Ӧ������ռ���ֽ���
    Char1 = Char - ' ';                                         // ����ƫ�ƺ��ֵ
    for (i = 0; i < Size2; i++)
    {
        if (Size == 8)
        {
            temp = OLED_ASCII0806[Char1][i];
        } // ����0806����
        else if (Size == 12)
        {
            temp = OLED_ASCII1206[Char1][i];
        } // ����1206����
        else if (Size == 16)
        {
            temp = OLED_ASCII1608[Char1][i];
        } // ����1608����
        else if (Size == 24)
        {
            temp = OLED_ASCII2412[Char1][i];
        } // ����2412����
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
 * @brief ��ʾ����
 * @param X ������ʾ����ʼX����
 * @param Y ������ʾ����ʼY����
 * @param Num Ҫ��ʾ������
 * @param Len ���ֵ�λ��
 * @param Size �����С
 * @param Color ��ʾ��ɫ
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

/**
 * @brief ��ʾͼƬ
 * @param X ͼƬ��ʾ����ʼX����
 * @param Y ͼƬ��ʾ����ʼY����
 * @param Sizex ͼƬ���
 * @param Sizey ͼƬ����
 * @param Image ͼƬ����
 * @param Color ͼƬ��ʾ����ɫ
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

/**
 * @brief ��ʾ���ֵ���
 * @param X ������ʾ����ʼX����
 * @param Y ������ʾ����ʼY����
 * @param Hanzi Ҫ��ʾ�ĺ��֣���Χ���ֿ��ַ�
 * @param Size ���ֵ������С
 * @param Color ��ʾ��ɫ
 */
void OLED_ShowChinese(u16 X, u16 Y, u8 *Hanzi, u8 Size, u16 Color) // ���ֵ��ִ�ӡ;
{
    u8 pIndex;
    if (Size == OLED_8X16)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi16x16[pIndex].Index, "") != 0; pIndex++)
        {
            /*�ҵ�ƥ��ĺ���*/
            if (strcmp(OLED_Hanzi16x16[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // ����ѭ������ʱpIndex��ֵΪָ�����ֵ�����
            }
        }
        /*��������ģ��OLED_Hanzi16x16��ָ��������16*16��ͼ���ʽ��ʾ*/
        OLED_ShowImage(X, Y, 16, 16, OLED_Hanzi16x16[pIndex].Data, Color);
    }
    else if (Size == OLED_6X12)
    {
        for (pIndex = 0; strcmp(OLED_Hanzi12x12[pIndex].Index, "") != 0; pIndex++)
        {
            /*�ҵ�ƥ��ĺ���*/
            if (strcmp(OLED_Hanzi12x12[pIndex].Index, (const char *)Hanzi) == 0)
            {
                break; // ����ѭ������ʱpIndex��ֵΪָ�����ֵ�����
            }
        }
        /*��������ģ��OLED_Hanzi12x12��ָ��������12*12��ͼ���ʽ��ʾ*/
        OLED_ShowImage(X, Y, 12, 12, OLED_Hanzi12x12[pIndex].Data, Color);
    }
}

/**
 * @brief ��ʾ�ַ���
 * @param X �ַ�����ʾ����ʼX����
 * @param Y �ַ�����ʾ����ʼY����
 * @param String Ҫ��ʾ���ַ�������Χ���ֿ��ַ���ɵ��ַ���
 * @param Size �ַ����������С
 * @param Color ��ʾ��ɫ
 */
void OLED_ShowString(u16 X, u16 Y, u8 *String, u8 Size, u16 Color) // ��Ӣ�Ĵ�ӡ;
{
    u8 i = 0, Len = 0, height = 0, width = 0;
    height = Size;
    switch (Size)
    {
    case OLED_6X8:
        width = 6;
        break;
    case OLED_6X12:
        width = 6;
        break;
    case OLED_8X16:
        width = 8;
        break;
    case OLED_12X24:
        width = 12;
        break;
    default:
        width = 6;
        break;
    }

    while (String[i] != '\0') // �����ַ�����ÿ���ַ�
    {
        if (String[i] == '\n')
        {
            Y += height;
            Len = 0;
            i++;
        } // ���ݻ��з�
        if ((X + (Len + 1) * width) > Paint.Height)
        {
            Y += height;
            Len = 0;
        } // ������Ļ�Զ�����
        if ((int8_t)Y > Paint.Width)
        {
            return;
        }

        if (String[i] > '~') // ���������Ӣ���ַ�
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
        else /*����OLED_ShowChar������������ʾÿ���ַ�*/
        {
            OLED_ShowChar(X + Len * width, Y, String[i], Size, Color);
            i++;
            Len++;
        }
    }
}

/**
 * @brief ʹ��printf������ӡ��ʽ���ַ���
 * @param X ��ʽ���ַ�����ʾ����ʼX����
 * @param Y ��ʽ���ַ�����ʾ����ʼY����
 * @param Width �ַ�����ʾ����Ŀ��
 * @param Height �ַ�����ʾ����ĸ߶�
 * @param Size ָ�������С
 * @param format ָ��Ҫ��ʾ�ĸ�ʽ���ַ�������Χ��ASCII��ɼ��ַ���ɵ��ַ���
 * @param ... ��ʽ���ַ��������б�
 */
void OLED_Printf(u16 X, u16 Y, u8 Size, u16 Color, const char *format, ...)
{
    u8 String[50];                              // �����ַ�����
    va_list arg;                                // ����ɱ�����б��������͵ı���arg
    va_start(arg, format);                      // ��format��ʼ�����ղ����б�arg����
    vsprintf((char *)String, format, arg);      // ʹ��vsprintf��ӡ��ʽ���ַ����Ͳ����б��ַ�������
    va_end(arg);                                // ��������arg
    OLED_ShowString(X, Y, String, Size, Color); // OLED��ʾ�ַ����飨�ַ������������ַ���
}