#include "vga.h"
#include "io.h"
#include "idt.h"
#include "functions.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define BYTES_PER_PIXEL 3
#define SCREEN_PITCH (SCREEN_WIDTH * BYTES_PER_PIXEL)

// void draw_color_test();
extern void keyboard_handler_asm();
extern void memory_copy(unsigned char* source, unsigned char* dest, int nbytes);

int cursor_x = 10;
int cursor_y = 50;
char cmd_buffer[64];
int cmd_idx = 0;
unsigned char* back_buffer;
volatile int input_ready = 0;   
char* current_input_ptr = 0;    
int input_max_len = 0;         


void dump_mem(unsigned int start_addr, int lines, int* cy) {
    unsigned char* ptr = (unsigned char*)start_addr; 
    char addr_str[12];
    char hex_val[3];
    
    for (int i = 0; i < lines; i++) {
        itoa_hex((unsigned int)ptr, addr_str); 
        kprint_str_gfx(addr_str, 10, *cy, 0xAAAAAA);

        for (int j = 0; j < 16; j++) {
            byte_to_hex(*ptr, hex_val);
            kprint_str_gfx(hex_val, 110 + (j * 20), *cy, 0xFFFFFF);
            ptr++;
        }
        *cy += 12;
    }
}

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

void dump_sector(unsigned short* buffer, int* cy) {
    char hex_val[3];
    unsigned char* byte_ptr = (unsigned char*)buffer;

    for (int line = 0; line < 32; line++) { 
        for (int j = 0; j < 16; j++) {
            byte_to_hex(*byte_ptr, hex_val);
            kprint_str_gfx(hex_val, 10 + (j * 20), *cy, 0xFFFFFF);
            byte_ptr++;
        }
        *cy += 12;
        
        if (*cy > (SCREEN_HEIGHT - 20)) break;
    }
}

void k_input(char* buffer, int max_len) {
    input_ready = 0;
    cmd_idx = 0;                  
    current_input_ptr = buffer;   
    input_max_len = max_len;
    while (!input_ready) {
        screen_update();
        __asm__ volatile("hlt"); 
    }

    for(int i = 0; i < cmd_idx; i++) {
        buffer[i] = cmd_buffer[i];
    }
    buffer[cmd_idx] = '\0';
    
    input_ready = 0;
}

void process_command(char* cmd, int* cy) {
    if (strcmp(cmd, "CLEAR")) {
        clear_screen_gfx();

        *cy = 10; 
    } 
    else if (strcmp(cmd, "TEST")) {
        draw_color_test();
        *cy += 20;
    }
    else if (strcmp(cmd, "READ")) {
        unsigned short* disk_buf = (unsigned short*)kmalloc(512);
        
        kprint_str_gfx("READING LBA0", 10, *cy, 0xFFFFFF);
        *cy += 14;

        ata_read_sector(0, disk_buf);

        dump_sector(disk_buf, cy);
        
        screen_update();
    }
    else if (strcmp(cmd, "WRITE")) {
        unsigned short* write_buf = (unsigned short*)kmalloc(512);
        // Wypełnij bufor testowymi danymi
        write_buf[0] = 0xADDE;
        write_buf[1] = 0xEFBE;
        for(int i=2; i<254; i++) write_buf[i] = 0x0000; 

        kprint_str_gfx("WRITING TO LBA0...", 10, *cy, 0xFFFFFF);
        ata_write_sector(0, write_buf);
        *cy += 14;
        kprint_str_gfx("DONE.", 10, *cy, 0xFFFFFF);
        *cy += 14;
    }
    else if (strcmp(cmd, "REBOOT")) {
        outb(0x64, 0xFE);
    }
    else if (strcmp(cmd, "SCAN")) {
        ata_identify(cy);
        screen_update();
    }
    else if (strcmp(cmd, "DUMP")) {
        //to jest po prostu dump z podanego adresu
        char msg[] = "dump";
        kprint_str_gfx(msg, 10, *cy, 0xFFFFFF);
        *cy += 14;
        dump_mem(0x100000, 40, cy);
    }
    else if (strcmp(cmd, "PEEK")) {
        //to też
        char msg[] = "dump";
        kprint_str_gfx(msg, 10, *cy, 0xFFFFFF);
        *cy += 14;     
        dump_mem((unsigned int)0x5000000, 10, cy);
    }
    else {
        char str[] = "UNKNOWN CMD";
        kprint_str_gfx(str, 10, *cy, 0xFF0000);
        *cy += 14;
    }
    if (*cy > (SCREEN_HEIGHT - 30)) {
        clear_screen_gfx();
        *cy = 10; 
    }
}

//to bym wywalił do vga.c ale zostawię
void refresh_cursor(int color) {
    draw_cursor(cursor_x, cursor_y, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

void keyboard_handler_c() {
    unsigned char scancode = inb(0x60);
    refresh_cursor(0x000000); 

    if (!(scancode & 0x80)) {
        char c = scancode_to_char(scancode);

        if (c == '\n') {
            // Jeśli jesteśmy w trybie "INPUT", ustawiamy flagę gotowości
            if (current_input_ptr != 0) {
                input_ready = 1;
                current_input_ptr = 0; // Czyścimy wskaźnik
            } else {
                // To co masz teraz - przetwarzanie komend
                cmd_buffer[cmd_idx] = '\0';
                cursor_x = 10;
                cursor_y += 14;
                if (cmd_idx > 0) process_command(cmd_buffer, &cursor_y);
                cmd_idx = 0;
            }
        } 
        else if (c == '\b') {
            if (cmd_idx > 0) {
                cursor_x -= 8;
                cmd_idx--;
                cmd_buffer[cmd_idx] = '\0';
                draw_rect(cursor_x, cursor_y, 8, 10, 0, 0, 0);
            }
        }
        else if (c > 0 && cmd_idx < 63) {
            cmd_buffer[cmd_idx++] = c;
            char char_str[2] = {c, '\0'};
            kprint_str_gfx(char_str, cursor_x, cursor_y, 0xFFFFFF);
            cursor_x += 8;
        }
    }
    refresh_cursor(0xFFFFFF);
}

void main() {
    back_buffer = (unsigned char*)kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
    //testy
    unsigned char* test_ptr = (unsigned char*)0x500000;
    test_ptr[0] = 0xDE;
    test_ptr[1] = 0xAD;
    test_ptr[2] = 0xBE;
    test_ptr[3] = 0xEF;
    test_ptr[4] = 0xCA;
    test_ptr[5] = 0xFE;
    clear_screen_gfx();
    idt_install();      
    pic_remap();       
    
    __asm__ volatile("sti");

    unsigned short low_kb = *(volatile unsigned short*)0x7000;
    unsigned short high_64kb = *(volatile unsigned short*)0x7004;
    unsigned int total_mb = (unsigned int)(low_kb / 1024) + ((unsigned int)high_64kb * 64 / 1024) + 1;

    char val_str[12];
    itoa(total_mb, val_str);

    kprint_str_gfx("vos 0.2 beta", 10, 10, 0xFFFFFF);
    kprint_str_gfx("RAM: ", 10, 30, 0xFFFFFF);
    kprint_str_gfx(val_str, 50, 30, 0xFFFFFF);
    kprint_str_gfx("MB", 80, 30, 0xFFFFFF);

    while(1) {
            // aktualizuj i haltuj
            screen_update();
            __asm__ volatile("hlt");
    }
}
// tego nie ruszam bo jak tam tego nie ma to linkerowi odwala i się restartuje
void __main() {}