#include "timer.h"
#include "io.h"
#include "vga.h"

volatile unsigned long timer_ticks = 0;
extern void timer_handler_asm();
extern void refresh_cursor(int color);

void timer_install(unsigned int frequency) {
    unsigned int divisor = 1193180 / frequency;

    outb(0x43, 0x36);             
    outb(0x40, (unsigned char)(divisor & 0xFF));   
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void timer_handler_c() {
    timer_ticks++;
    outb(0x20, 0x20); 
    if (timer_ticks % 100 == 0) {
        refresh_cursor(0xFFFFFF); 
    } else if (timer_ticks % 100 == 50) {
        refresh_cursor(0x000000); 
    }
}

unsigned long long int get_timer_tick() {
    return timer_ticks;
}