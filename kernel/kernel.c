#include "vga.h"
#include "io.h"
#include "idt.h"
#include "ata.h"
#include "timer.h"
#include "functions.h"
#include "rtc.h"
#include "icons.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 960
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

int window_running = 0;

void format_two_digits(int val, char* buf) {
    buf[0] = (val / 10) + '0'; // Cyfra dziesiątek
    buf[1] = (val % 10) + '0'; // Cyfra jedności
    buf[2] = '\0';             // Terminator
}

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

void draw_window(int width, int height, int bar_height, int x, int y, char* text) {
    draw_rect(x, y, width+2, height, 255, 255, 255);
    draw_rect(x+1, y+21, width, height-bar_height-2, 0, 0, 0);
    // rysuj jakiś tam gradient
    for (int i = 0; i < bar_height+1; i++) {
        draw_rect(x+1, y+1+i, width, 1, 100+4*i, 100+4*i, 100+4*i);
    }
    // draw_rect(x+1, y+22, width, 1, 255, 255, 255);
    kprint_str_gfx(text, x+3, y+7, 0xFFFFFF);
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
    else if (strcmp(cmd, "WIN")) {
        window_running = 1;
        fill_screen_fast(0, 85, 170);
        draw_icon(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 24, 24, fdicon);
        draw_icon((SCREEN_WIDTH/2)+30, SCREEN_HEIGHT/2, 24, 24, folder_icon);
        
        draw_window(800, 600, 20, 29, 29, "Program manager");
    }
    else if (strcmp(cmd, "GETRTC")) {
        int time_zone = 2;

        char hour[3];
        char min[3];
        char sec[3];
        int h, m, s; 
        read_rtc(&s, &m, &h); // ADRESY
        int actual_h = (h + time_zone)%24;
        
        format_two_digits(actual_h, hour);
        format_two_digits(m, min);
        format_two_digits(s, sec);
        kprint_str_gfx(hour, cursor_x, cursor_y, 0xFFFFFF);
        kprint_str_gfx(":", cursor_x+20, cursor_y, 0xFFFFFF);
        kprint_str_gfx(min, cursor_x+30, cursor_y, 0xFFFFFF);
        kprint_str_gfx(":", cursor_x+50, cursor_y, 0xFFFFFF);
        kprint_str_gfx(sec, cursor_x+60, cursor_y, 0xFFFFFF);
        cursor_y +=14;
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

// to bym wywalił do vga.c bo to funkcja czysto wideo
// tylko że ta "funkcja czysto wideo" opiera się na liniach 17 i 18 a one są tutaj i mi się nie chce robić externów
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
    if (scancode == 0x01) { // ESC pressed
        window_running = 0;
        clear_screen_gfx();
        cursor_x = 10;
        cursor_y = 10;
        return;
    }
    refresh_cursor(0xFFFFFF);
}

void main() {
    back_buffer = (unsigned char*)kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
    clear_screen_gfx();
    idt_install();      
    pic_remap();
    timer_install(100);
    
    asm volatile("sti");

    unsigned short low_kb = *(volatile unsigned short*)0x7000;
    unsigned short high_64kb = *(volatile unsigned short*)0x7004;
    unsigned int total_mb = (unsigned int)(low_kb / 1024) + ((unsigned int)high_64kb * 64 / 1024) + 1;

    char val_str[12];
    itoa(total_mb, val_str);

    kprint_str_gfx("vos 0.3 beta ---", 10, 10, 0xFFFFFF);
    kprint_str_gfx("RAM: ", 10, 30, 0xFFFFFF);
    kprint_str_gfx(val_str, 50, 30, 0xFFFFFF);
    kprint_str_gfx("MB", 80, 30, 0xFFFFFF);
    
    for (int k = 0; k < 7; k++) {
        switch(k) {
            case 0: draw_rect(150+10*k, 10, 10, 10, 255, 0, 0); break;
            case 1: draw_rect(150+10*k, 10, 10, 10, 0, 255, 0); break;
            case 2: draw_rect(150+10*k, 10, 10, 10, 0, 0, 255); break;
            case 3: draw_rect(150+10*k, 10, 10, 10, 255, 255, 0); break;
            case 4: draw_rect(150+10*k, 10, 10, 10, 0, 255, 255); break;
            case 5: draw_rect(150+10*k, 10, 10, 10, 255, 0, 255); break;
            case 6: draw_rect(150+10*k, 10, 10, 10, 255, 255, 255); break;
        }
    }

    while(1) {
            // aktualizuj i haltuj
            screen_update();
            asm volatile("hlt");
    }
}
// tego nie ruszam bo jak tam tego nie ma to linkerowi odwala i się restartuje
void __main() {}