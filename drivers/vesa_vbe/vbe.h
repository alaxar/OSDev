#ifndef _VBE_H_
#define _VBE_H_

#include "../../include/types.h"

typedef struct vbe_info_structure {
    char signature[4];
    uint16_t version;
    uint32_t oem;
    uint32_t capabilities;
    uint32_t video_modes;
    uint16_t video_memory;
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256]; 
} __attribute__((packed)) vbe_info_structure;

typedef struct vbe_mode_info_struture {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;             // number of bytes per horizontal line
    uint16_t width;             // width in pixels
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__((packed)) vbe_mode_info_struture;

// external varibles
extern struct vbe_mode_info_struture *vbe_mode;
extern struct vbe_info_structure *vbe;

int vbe_software_support();
void get_vbe_mode_info();
#endif