#include "vga.h"
#include "io.h"

// I TAK NIC Z TEGO NIE JEST UŻYWANE BO PRZESZEDŁEM NA TRYB GRAFICZNY

// stałe

#define MAX_ROWS 25
#define MAX_COLS 80
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define BYTES_PER_PIXEL 3
#define SCREEN_PITCH (SCREEN_WIDTH * BYTES_PER_PIXEL)

//to jest do clear screen
extern void memory_copy(unsigned char* source, unsigned char* dest, int nbytes);

void screen_update() {
    unsigned char* lfb = (unsigned char*)0xFD000000;
    memory_copy(back_buffer, lfb, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
}