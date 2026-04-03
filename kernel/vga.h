#ifndef VGA_H
#define VGA_H

//dawne vga funkcje nie używane wyjebałem je z vga.c
extern unsigned char* back_buffer;
void screen_update();
void put_pixel(int x, int y, int r, int g, int b);
void draw_char(int x, int y, unsigned char font_chars[10], int color);
void clear_screen_gfx();
void kprint_char_gfx(char c, int x, int y, int color) ;
#endif