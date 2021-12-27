/*
*    http://karig.net/os/001d.html
*
*    use monochrome bmp file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    /* Open font.bmp for binary read, output font.bin */
    /* Bitmap must be monochrome */
    char* buffer;

    FILE* font = fopen("font.bmp","rb");
    FILE* bin = fopen("font.bin","wb");
    buffer =(char*) malloc(4096);
    fread(buffer, 1, 62, font);
    fread(buffer, 1, 4096, font);
    fclose(font);
    for (int i = 0; i < 256; ++i) {
        for (int j = 15; j >= 0; --j) {
            fwrite(buffer+(j*256)+i, 1, 1, bin);
        }
    }
    fclose(bin);
    free(buffer);
    return 0;
}
