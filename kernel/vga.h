#ifndef VGA_H
#define VGA_H

//dawne vga funkcje nie używane wyjebałem je z vga.c
extern unsigned char* back_buffer;
void screen_update();
void put_pixel(int x, int y, int r, int g, int b);
void draw_char(int x, int y, unsigned char font_chars[10], int color);
void clear_screen_gfx();
void kprint_char_gfx(char c, int x, int y, int color);
void kprint_str_gfx(char* str, int x_start, int y_start, int color);
void draw_rect(int start_x, int start_y, int width, int height, int r, int g, int b);
void draw_color_test();
void draw_cursor(int x, int y, int r, int g, int b);
void fill_screen_fast(int r, int g, int b);
void draw_icon(int start_x, int start_y, int w, int h, unsigned int* icon_data);
void draw_window(int width, int height, int bar_height, int x, int y, char* text);
#endif