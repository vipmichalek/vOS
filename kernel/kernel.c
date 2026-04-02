#include "vga.h"
#include "io.h"
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define BYTES_PER_PIXEL 3
#define SCREEN_PITCH (SCREEN_WIDTH * BYTES_PER_PIXEL)

void draw_color_test();
void idt_install();
void pic_remap();
extern void keyboard_handler_asm();
extern void memory_copy(unsigned char* source, unsigned char* dest, int nbytes);

int cursor_x = 10;
int cursor_y = 50;
char cmd_buffer[64];
int cmd_idx = 0;
unsigned int heap_current = 0x200000; 
unsigned char* back_buffer;
unsigned int next_rand = 1;

// jednak zrobiłem
void* kmalloc(unsigned int size) {
    void* addr = (void*)heap_current;
    heap_current += size;

    if (heap_current % 4 != 0) {
        heap_current += (4 - (heap_current % 4));
    }
    
    return addr;
}

int rand() {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}



struct idt_entry {
    unsigned short base_low;
    unsigned short selector;      
    unsigned char  zero;          
    unsigned char  flags;        
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void pic_remap() {
    outb(0x20, 0x11); // Start initialization
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC vector offset (0x20)
    outb(0xA1, 0x28); // Slave PIC vector offset (0x28)
    outb(0x21, 0x04); // Tell Master there is a Slave at IRQ2
    outb(0xA1, 0x02); // Tell Slave its identity
    outb(0x21, 0x01); // 8086 mode
    outb(0xA1, 0x01);
    outb(0x21, 0xFD); // 0xFD = 11111101 (Masks everything except IRQ1 - Keyboard)
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

    idt_set_gate(33, (unsigned long)keyboard_handler_asm, 0x08, 0x8E);

    __asm__ volatile("lidt (%0)" : : "r" (&idtp));
}

// tu użyłem gemini bo mi się nie chciało
unsigned char font8x10_basic[59][10] = {
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00}, // A - cleaner bar
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00}, // B - better symmetry
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // E - fixed middle bar
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00}, // F
    {0x3C, 0x66, 0x60, 0x60, 0x6E, 0x66, 0x66, 0x66, 0x3E, 0x00}, // G
    {0x66, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00}, // H
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00}, // J
    {0x66, 0x6C, 0x78, 0x70, 0x60, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x7E, 0x00}, // L
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00}, // M
    {0x66, 0x76, 0x7E, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x66, 0x00}, // N
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
    {0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x00}, // P
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06, 0x07}, // Q - fixed tail
    {0x7C, 0x66, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x66, 0x00}, // R
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x06, 0x66, 0x66, 0x3C, 0x00}, // S - smoother curves
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00}, // V
    {0x63, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x63, 0x00}, // W
    {0x66, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x00}, // X
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // Y
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x40, 0x7E, 0x00}, // Z
    {0x3C, 0x66, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 0
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // 1
    {0x3C, 0x66, 0x06, 0x06, 0x3C, 0x60, 0x60, 0x66, 0x7E, 0x00}, // 2 - clearer base
    {0x3C, 0x66, 0x06, 0x06, 0x1C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 3
    {0x06, 0x0E, 0x1E, 0x36, 0x66, 0x7F, 0x06, 0x06, 0x06, 0x00}, // 4
    {0x7E, 0x60, 0x60, 0x7C, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 5
    {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 6
    {0x7E, 0x06, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 7
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // 8
    {0x3C, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0C, 0x38, 0x00},  // 9
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // . (Kropka) - indeks 36
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30}, // , (Przecinek) - indeks 37
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00}, // / (RSlash) - indeks 38
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00}, // \ (LSlash) - indeks 39
    {0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00}, // - (Myślnik) - indeks 40
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, // ( (Nawias L) - indeks 41
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, // ) (Nawias R) - indeks 42
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00},
};

//tu tueż
char scancode_to_char(unsigned char scancode) {
    switch(scancode) {
        case 0x0E: return '\b'; case 0x1C: return '\n';
        case 0x1E: return 'A'; case 0x30: return 'B'; case 0x2E: return 'C';
        case 0x20: return 'D'; case 0x12: return 'E'; case 0x21: return 'F';
        case 0x22: return 'G'; case 0x23: return 'H'; case 0x17: return 'I';
        case 0x24: return 'J'; case 0x25: return 'K'; case 0x26: return 'L';
        case 0x32: return 'M'; case 0x31: return 'N'; case 0x18: return 'O';
        case 0x19: return 'P'; case 0x10: return 'Q'; case 0x13: return 'R';
        case 0x1F: return 'S'; case 0x14: return 'T'; case 0x16: return 'U';
        case 0x2F: return 'V'; case 0x11: return 'W'; case 0x2D: return 'X';
        case 0x15: return 'Y'; case 0x2C: return 'Z'; case 0x39: return ' ';
        case 0x0B: return '0'; case 0x02: return '1'; case 0x03: return '2';
        case 0x04: return '3'; case 0x05: return '4'; case 0x06: return '5';
        case 0x07: return '6'; case 0x08: return '7'; case 0x09: return '8';
        case 0x0A: return '9';
        default: return 0;
    }
}

int strcmp(char* s1, char* s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 1;
        i++;
    }
    return 0;
}

void screen_update() {
    unsigned char* lfb = (unsigned char*)0xFD000000;
    memory_copy(back_buffer, lfb, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
}

void put_pixel(int x, int y, int r, int g, int b) {
    int offset = (y * SCREEN_PITCH) + (x * BYTES_PER_PIXEL);
    
    back_buffer[offset]     = b; 
    back_buffer[offset + 1] = g; 
    back_buffer[offset + 2] = r; 
}

void draw_char(int x, int y, unsigned char font_chars[10], int color) {
    for (int i = 0; i < 10; i++) {     
        for (int j = 0; j < 8; j++) {
            if (font_chars[i] & (1 << (7 - j))) {
                int r = (color >> 16) & 0xFF;
                int g = (color >> 8) & 0xFF;
                int b = color & 0xFF;
                put_pixel(x + j, y + i, r, g, b);
            }
        }
    }
}

void clear_screen_gfx() {
    for(int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL); i++) {
        back_buffer[i] = 0;
    }
}

void kprint_char_gfx(char c, int x, int y, int color) {
    if (c >= 'a' && c <= 'z') c -= 32;

    if (c >= 'A' && c <= 'Z') {
        draw_char(x, y, font8x10_basic[c - 'A'], color);
    } 
    else if (c >= '0' && c <= '9') {
        draw_char(x, y, font8x10_basic[26 + (c - '0')], color);
    }
    // Obsługa znaków specjalnych
    else {
        int index = -1;
        switch(c) {
            case '.':  index = 36; break;
            case ',':  index = 37; break;
            case '/':  index = 38; break;
            case '\\': index = 39; break;
            case '-':  index = 40; break;
            case '(':  index = 41; break;
            case ')':  index = 42; break;
            case ':':  index = 43; break;
        }

        if (index != -1) {
            draw_char(x, y, font8x10_basic[index], color);
        }
    }
}

void kprint_str_gfx(char* str, int x_start, int y_start, int color) {
    int current_x = x_start;
    int current_y = y_start;
    int i = 0;

    while (str[i] != '\0') {
        char c = str[i];

        if (c == '\n') {
            // POWRÓT KARETKI I NOWA LINIA
            current_x = x_start;  
            current_y += 10;      
        } 
        else if (c == ' ') {
            // SPACJA
            current_x += 8;       // w prawo
        } 
        else {
            // ZWYKŁY ZNAK
            kprint_char_gfx(c, current_x, current_y, color);
            current_x += 8;       
        }

        // wraparound
        if (current_x > SCREEN_WIDTH - 20) { 
            current_x = x_start;
            current_y += 10;
        }

        i++;
    }
}

void hex_to_str(unsigned char n, char* out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = '0';
    out[1] = 'X';
    out[2] = hex_chars[(n >> 4) & 0x0F];
    out[3] = hex_chars[n & 0x0F];
    out[4] = '\0';
}

void byte_to_hex(unsigned char n, char* out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = hex_chars[(n >> 4) & 0x0F];
    out[1] = hex_chars[n & 0x0F];
    out[2] = '\0';
}

void itoa_hex(unsigned int n, char* str) {
    const char hex_chars[] = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        str[i + 2] = hex_chars[n & 0xF];
        n >>= 4;
    }
    str[10] = '\0';
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

void draw_rect(int start_x, int start_y, int width, int height, int r, int g, int b) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            put_pixel(x, y, r, g, b);
        }
    }
}

void draw_cursor(int x, int y, int r, int g, int b) {
    draw_rect(x, y+10, 8, 2, r, g, b);
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
        kprint_str_gfx(msg, 10, *cy, 0xFFFF00);
        *cy += 14;
        dump_mem(0x100000, 40, cy);
    }
    else if (strcmp(cmd, "PEEK")) {
        //to też
        char msg[] = "dump";
        kprint_str_gfx(msg, 10, *cy, 0x00FFFF);
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

void draw_color_test() {
    int start_y = 100;
    int bar_height = 50;
    int spacing = 0;
    //paski
    for (int i = 0; i < 256; i++) {
        draw_rect(100 + (i * 2), start_y, 2, bar_height, i, 0, 0);
        draw_rect(100 + (i * 2), start_y + bar_height + spacing, 2, bar_height, 0, i, 0);
        draw_rect(100 + (i * 2), start_y + (bar_height + spacing) * 2, 2, bar_height, 0, 0, i);
        draw_rect(100 + (i * 2), start_y + (bar_height + spacing) * 3, 2, bar_height, i, i, 0);
        draw_rect(100 + (i * 2), start_y + (bar_height + spacing) * 4, 2, bar_height, 0, i, i);
        draw_rect(100 + (i * 2), start_y + (bar_height + spacing) * 5, 2, bar_height, i, 0, i);
        draw_rect(100 + (i * 2), start_y + (bar_height + spacing) * 6, 2, bar_height, i, i, i);
    }
}

void itoa(unsigned int n, char* str) {
    int i;
    int len = 0;
    unsigned int temp = n;
    if (n == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (temp > 0) {
        temp /= 10;
        len++;
    }
    str[len] = '\0';

    for (i = len - 1; i >= 0; i--) {
        str[i] = (n % 10) + '0';
        n /= 10;
    }
}

void refresh_cursor(int color) {
    draw_cursor(cursor_x, cursor_y, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

void keyboard_handler_c() {
    unsigned char scancode = inb(0x60);
    refresh_cursor(0x000000); 

    if (!(scancode & 0x80)) {
        char c = scancode_to_char(scancode);

        if (c == '\b') { // Backspace
            if (cmd_idx > 0) {
                cursor_x -= 8;
                cmd_idx--;
                cmd_buffer[cmd_idx] = '\0';
                draw_rect(cursor_x, cursor_y, 8, 10, 0, 0, 0);
            }
        } 
        else if (c == '\n') { // Enter
            cmd_buffer[cmd_idx] = '\0'; 
            cursor_x = 10;
            cursor_y += 14;
            
            if (cmd_idx > 0) {
                process_command(cmd_buffer, &cursor_y);
            }
            cmd_idx = 0; 
        }
        else if (c > 0 && cmd_idx < 63) { // znak
            cmd_buffer[cmd_idx++] = c;
            char char_str[2] = {c, '\0'};
            kprint_str_gfx(char_str, cursor_x, cursor_y, 0xFFFFFF);
            cursor_x += 8;
        }
    }
    refresh_cursor(0xFFFFFF);
    if (cursor_y > (SCREEN_HEIGHT - 20)) {
        clear_screen_gfx();
        cursor_y = 10;
    }
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

    char text[] = "vos 0.2 beta"; 

    unsigned short low_kb = *(volatile unsigned short*)0x7000;
    unsigned short high_64kb = *(volatile unsigned short*)0x7004;
    unsigned int total_mb = (unsigned int)(low_kb / 1024) + ((unsigned int)high_64kb * 64 / 1024) + 1;

    char val_str[12];
    itoa(total_mb, val_str);

    kprint_str_gfx(text, 10, 10, 0xFFFFFF);
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
