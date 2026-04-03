#include "ata.h"
#include "vga.h"
#include "io.h"

void ata_identify(int* cy) {
    unsigned short info[256];
    char model[41]; // 40 chars + null terminator

    // 1. Select Master (0xA0) or Slave (0xB0)
    // For now, let's just check the Primary Master
    outb(0x1F6, 0xA0); 
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC); // IDENTIFY

    unsigned char status = inb(0x1F7);
    if (status == 0) {
        kprint_str_gfx("NO IDE DRIVE FOUND", 10, *cy, 0xFF0000);
        *cy += 14;
        return;
    }

    // Wait for BSY to clear and DRQ to set
    while (inb(0x1F7) & 0x80); 
    while (!(inb(0x1F7) & 0x08)); 

    // 2. Read the 512 bytes into our array
    for (int i = 0; i < 256; i++) {
        info[i] = inw(0x1F0);
    }

    // 3. Extract Model Name (Words 27-46)
    for (int i = 0; i < 20; i++) {
        unsigned short word = info[27 + i];
        model[i * 2] = (char)(word >> 8);   // High byte first
        model[i * 2 + 1] = (char)(word & 0xFF); // Low byte second
    }
    model[40] = '\0';

    // 4. Print it!
    kprint_str_gfx("FOUND DISK: ", 10, *cy, 0x00FF00);
    kprint_str_gfx(model, 110, *cy, 0xFFFFFF);
    *cy += 14;
}

void ata_read_sector(unsigned int lba, unsigned short* buffer) {
    outb(0x1F6, (0xE0 | ((lba >> 24) & 0x0F))); 
    outb(0x1F2, 1);                             
    outb(0x1F3, (unsigned char)lba);            
    outb(0x1F4, (unsigned char)(lba >> 8));     
    outb(0x1F5, (unsigned char)(lba >> 16));    
    outb(0x1F7, 0x20);                          

    while (inb(0x1F7) & 0x80); 
    while (!(inb(0x1F7) & 0x08)); 

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0);
    }
}

void ata_write_sector(unsigned int lba, unsigned short* buffer) {
    outb(0x1F6, (0xE0 | ((lba >> 24) & 0x0F))); 
    outb(0x1F2, 1);                             
    outb(0x1F3, (unsigned char)lba);            
    outb(0x1F4, (unsigned char)(lba >> 8));     
    outb(0x1F5, (unsigned char)(lba >> 16));    
    outb(0x1F7, 0x30);                          
    while (inb(0x1F7) & 0x80); 
    while (!(inb(0x1F7) & 0x08)); 
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]);
    }
    outb(0x1F7, 0xE7);
    while (inb(0x1F7) & 0x80); 
}