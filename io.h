#ifndef IO_H
#define IO_H

//outb i inb i tutaj musze dodać inne rzeczy

void outb(unsigned short port, unsigned char val);
unsigned char inb(unsigned short port);
static inline unsigned short inw(unsigned short port) {
    unsigned short result;
    asm volatile("inw %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static inline void outw(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (port));
}

#endif