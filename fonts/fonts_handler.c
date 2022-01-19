#include "fonts_handler.h"
#include "../drivers/screen.h"

extern char *_binary_Untitled_bmp_start;
// extern char _binary_fonts_zap_vga16_psf_start;
extern char _binary_fonts_zap_ext_vga16_psf_start;

int psf_init() {
    // struct bmp_header *font_header = (struct bmp_header*)_binary_Untitled_bmp_start;

    // if(font_header->type != 0x4d42) {
    //     printf("Hooraay", -1, -1);
    // }
    // struct dib_header *dib = (struct dib_header*)_binary_Untitled_bmp_start + 14;


    // if(dib->headerSize == 40) {
    //     printf("yee", -1, -1);
    // }
}