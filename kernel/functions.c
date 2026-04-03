unsigned int next_rand = 1;
unsigned int heap_current = 0x200000; 

int rand() {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}

int strcmp(char* s1, char* s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 1;
        i++;
    }
    return 0;
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

void* kmalloc(unsigned int size) {
    void* addr = (void*)heap_current;
    heap_current += size;

    if (heap_current % 4 != 0) {
        heap_current += (4 - (heap_current % 4));
    }
    
    return addr;
}

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