#include "timer.h"

volatile unsigned long timer_ticks = 0;
extern void timer_handler_asm();

void timer_install(unsigned int frequency) {
    unsigned int divisor = 1193180 / frequency;

    outb(0x43, 0x36);             
    outb(0x40, (unsigned char)(divisor & 0xFF));   
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void timer_handler_c() {
    timer_ticks++;
    outb(0x20, 0x20); 
    // TO WYJEBAĆ
    // TO JEST TEST TIMERA
    if (timer_ticks % 10 == 0) {
        draw_rect(0, 0, 5, 5, 0, 255, 0); 
    } else if (timer_ticks % 10 == 5) {
        draw_rect(0, 0, 5, 5, 0, 0, 0);
    }
}