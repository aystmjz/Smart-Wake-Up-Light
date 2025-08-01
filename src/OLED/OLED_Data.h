#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH 3 // UTF-8编码格式给3，GB2312编码格式给2

#define OLED_UNICODE_8X16FONT_ADDR 0x000000 // Unicode字库地址 0x000000 ~ 0x200000 (U+0000 ~ U+FFFF 大小2MB)
#define OLED_UNICODE_6X12FONT_ADDR 0x200000 // Unicode字库地址 0x200000 ~ 0x380000 (U+0000 ~ U+FFFF 大小1.5MB)

#define OLED_USE_RLE

/*字模基本单元*/
typedef struct
{
	uint8_t Data[32];					 // 字模数据
	char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
} ChineseCell16x16_t;

typedef struct
{
	uint8_t Data[24];					 // 字模数据
	char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
} ChineseCell12x12_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_ASCII0806[][6];
extern const uint8_t OLED_ASCII1206[][12];
extern const uint8_t OLED_ASCII1608[][16];
extern const uint8_t OLED_ASCII2412[][36];

#ifdef OLED_USE_RLE
extern const uint16_t OLED_ASCII10452_RLE_index[13];
extern const uint8_t OLED_ASCII10452_RLE[1592];
#else
extern const uint8_t OLED_ASCII10452[][780];
#endif

/*汉字字模数据声明*/
extern const ChineseCell16x16_t OLED_Hanzi16x16[];
extern const ChineseCell12x12_t OLED_Hanzi12x12[];

/*图像数据声明*/
#ifdef OLED_USE_RLE
extern const uint16_t Image_RLE_index[2];
extern const uint8_t Image_RLE[1242];
#else
extern const uint8_t Image_1[4796];
#endif

#endif
