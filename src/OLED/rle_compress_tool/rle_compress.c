#include <stdio.h>
#include <stdint.h>
#include "rle_compress_data.h"

#define CHAR_COUNT 12 // 字符数量，根据实际调整
#define CHAR_SIZE 780 // 每个字符的字节大小，根据实际调整
#define CHAR_DATA OLED_ASCII10452

// 游程压缩函数
void rle_compress_char(const uint8_t *input, int input_len, uint8_t *output, int *out_len)
{
    int i = 0, j = 0;
    while (i < input_len)
    {
        uint8_t val = input[i];
        int run_len = 1;
        while (i + run_len < input_len && input[i + run_len] == val && run_len < 255)
        {
            run_len++;
        }
        output[j++] = val;
        output[j++] = (uint8_t)run_len;
        i += run_len;
    }
    *out_len = j;
}

int main()
{
    FILE *fp = fopen("output/oled_font_rle.txt", "w");
    if (!fp)
    {
        perror("Failed to open output file");
        return 1;
    }

    printf("Start RLE compression of font data...\n");
    fprintf(fp, "/* RLE compressed font data, character count: %d, each char size: %d bytes */\n\n", CHAR_COUNT, CHAR_SIZE);

    // 用于拼接总数据和构建索引
    uint8_t all_data[CHAR_COUNT * CHAR_SIZE * 2] = {0};
    uint32_t all_offset[CHAR_COUNT] = {0};
    uint32_t total_len = 0;

    for (int c = 0; c < CHAR_COUNT; c++)
    {
        uint8_t compressed[CHAR_SIZE * 2];
        int compressed_len = 0;

        rle_compress_char(CHAR_DATA[c], CHAR_SIZE, compressed, &compressed_len);

        printf("Compressed char index %d: original size %d, compressed size %d\n", c, CHAR_SIZE, compressed_len);

        fprintf(fp, "/* Char index %d, compressed length %d */\n", c, compressed_len);
        fprintf(fp, "const uint8_t oled_font_rle_data_%d[%d] = {", c, compressed_len);
        for (int i = 0; i < compressed_len; i++)
        {
            if (i % 16 == 0)
                fprintf(fp, "\n    ");
            fprintf(fp, "0x%02X,", compressed[i]);
        }
        fprintf(fp, "\n};\n\n");

        // 添加到总数组中，并记录偏移
        all_offset[c] = total_len;
        for (int i = 0; i < compressed_len; i++)
        {
            all_data[total_len++] = compressed[i];
        }
    }

    // 输出总数据数组
    fprintf(fp, "/* Combined RLE data array */\n");
    fprintf(fp, "const uint8_t oled_font_rle_all[%d] = {", total_len);
    for (uint32_t i = 0; i < total_len; i++)
    {
        if (i % 16 == 0)
            fprintf(fp, "\n    ");
        fprintf(fp, "0x%02X,", all_data[i]);
    }
    fprintf(fp, "\n};\n\n");

    fprintf(fp, "/* Index array: [0] = count, [1..n-1] = offset[n], [n] = total_len */\n");
    fprintf(fp, "const uint16_t oled_font_rle_index[%d] = {", CHAR_COUNT + 1);

    // 第一个元素存字符数量
    fprintf(fp, "\n    %d,", CHAR_COUNT);

    // 从第1个字符（下标为1）开始写入偏移量（跳过0）
    for (int i = 1; i < CHAR_COUNT; i++)
    {
        if (i % 16 == 0)
            fprintf(fp, "\n    ");
        fprintf(fp, "%d,", all_offset[i]);
    }

    // 最后一个元素为总长度（最后字符的结束位置）
    fprintf(fp, "%d", total_len);
    fprintf(fp, "\n};\n\n");

    fclose(fp);
    printf("RLE compression done. Output file: output/oled_font_rle.txt\n");
    return 0;
}
