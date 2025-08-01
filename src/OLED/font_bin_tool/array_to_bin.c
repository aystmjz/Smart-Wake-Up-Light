#include <stdio.h>
#include <stdlib.h>
#include "font_data.h"

int main()
{
    FILE *fp = fopen("output/font_data.bin", "wb");
    if (!fp)
    {
        perror("Failed to open output file");
        return 1;
    }

    size_t written = 0;
    written += fwrite(Image_1, 1, Image_1_Size, fp);

    fclose(fp);
    printf("Successfully wrote %zu bytes to output/font_data.bin\n", written);
    return 0;
}
