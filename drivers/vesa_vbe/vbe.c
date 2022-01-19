#include "vbe.h"

unsigned char *vbe_info_pointer = 0x3000;
unsigned char *vbe_mode_info_pointer = 0x3512;

struct vbe_info_structure *vbe;  // this will point to the structure of vbe_info by the bootloader form 0x3000 memory address
struct vbe_mode_info_struture *vbe_mode;

int load_vbe_data_structures() {
    vbe = (struct vbe_info_structure*)vbe_info_pointer;  // this will point to the structure of vbe_info by the bootloader form 0x3000 memory address
    vbe_mode = (struct vbe_mode_info_struture*)vbe_mode_info_pointer;
}

int vbe_software_support() {

    // check whether bios supports vbe or not
    if(string_compare(vbe->signature, "VESA") == 0) {
        printf("BIOS doesnt support VBE at all\n", -1, -1);
    }
    printf("Signature: ", -1, -1);
    printf(vbe->signature, -1, -1);
}

void get_vbe_mode_info() {
    printf("\nWidth: ", -1, -1);
    printf(itoa(vbe_mode->width), -1, -1);
    printf("\nHeight: ", -1, -1);
    printf(itoa(vbe_mode->height), -1, -1);
}