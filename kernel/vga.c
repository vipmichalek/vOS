#include "vga.h"
#include "io.h"

// I TAK NIC Z TEGO NIE JEST UŻYWANE BO PRZESZEDŁEM NA TRYB GRAFICZNY

// stałe

#define MAX_ROWS 25
#define MAX_COLS 80
#define VIDEO_ADDRESS 0xb8000
#define WHITE_ON_BLACK 0x0F
#define RED_ON_BLACK   0x0C
#define GREEN_ON_BLACK 0x0A
#define YELLOW_ON_BLACK 0x0E

void update_cursor(int x, int y) {
    unsigned short pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void clear_screen() {
    char* video_memory = (char*) VIDEO_ADDRESS;
    for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        video_memory[i * 2] = ' '; 
        video_memory[i * 2 + 1] = WHITE_ON_BLACK; 
    update_cursor(0, 0);
    }
}
void kprint_at(char* message, int col, int row, unsigned char color) {
    char* video_memory = (char*) VIDEO_ADDRESS;
    int offset = (row * MAX_COLS + col) * 2;
    int i = 0;

    while (message[i] != '\0') {
        if (offset >= MAX_ROWS * MAX_COLS * 2) break;

        video_memory[offset] = message[i];
        video_memory[offset + 1] = color;     
        
        offset += 2;
        i++;
    }
    
    update_cursor(col + i, row);
}

void scroll() {
    char* video_memory = (char*) VIDEO_ADDRESS;

    for (int i = 1; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS * 2; j++) {
            video_memory[((i - 1) * MAX_COLS * 2) + j] = video_memory[(i * MAX_COLS * 2) + j];
        }
    }

    int last_line_offset = (MAX_ROWS - 1) * MAX_COLS * 2;
    for (int j = 0; j < MAX_COLS; j++) {
        video_memory[last_line_offset + (j * 2)] = ' ';
        video_memory[last_line_offset + (j * 2) + 1] = WHITE_ON_BLACK;
    }
}