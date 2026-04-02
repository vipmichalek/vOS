#ifndef VGA_H
#define VGA_H

// to jest właściwie nieużywane odkąd tryb graficzny

void clear_screen();
void scroll();
void update_cursor(int x, int y);
void kprint_at(char* message, int col, int row, unsigned char color);

#endif
