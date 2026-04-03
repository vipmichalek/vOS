#include "idt.h"
#include "io.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

volatile unsigned long timer_ticks = 0;

extern void keyboard_handler_asm();
extern void timer_handler_asm();
void pic_remap() {
    outb(0x20, 0x11); // Start initialization
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC vector offset (0x20)
    outb(0xA1, 0x28); // Slave PIC vector offset (0x28)
    outb(0x21, 0x04); // Tell Master there is a Slave at IRQ2
    outb(0xA1, 0x02); // Tell Slave its identity
    outb(0x21, 0x01); // 8086 mode
    outb(0xA1, 0x01);
    outb(0x21, 0xFC); // 0xFD = 11111101 (Masks everything except IRQ1 - Keyboard)
    outb(0xA1, 0xFF); // Mask all slave interrupts
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;
    
    for(int i=0; i<256; i++) idt_set_gate(i, 0, 0, 0);
    idt_set_gate(32, (unsigned long)timer_handler_asm, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)keyboard_handler_asm, 0x08, 0x8E);

    asm volatile("lidt (%0)" : : "r" (&idtp));
}

void timer_install(unsigned int frequency) {
    unsigned int divisor = 1193180 / frequency;

    outb(0x43, 0x36);             
    outb(0x40, (unsigned char)(divisor & 0xFF));   
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void timer_handler_c() {
    timer_ticks++;
    outb(0x20, 0x20); 
}