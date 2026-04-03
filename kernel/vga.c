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

//to jest do clear screen
extern void memory_copy(unsigned char* source, unsigned char* dest, int nbytes);
