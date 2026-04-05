#include "rtc.h"
#include "io.h"

#define BCD_TO_BIN(val) (((val & 0xF0) >> 4) * 10 + (val & 0x0F))

unsigned char get_update_in_progress_flag() {
      outb(0x70, 0x0A);
      return (inb(0x71) & 0x80);
}

unsigned char get_rtc_register(int reg) {
      outb(0x70, reg);
      return inb(0x71);
}

void read_rtc(int* second, int* minute, int* hour) {
    // czekaj
    while (get_update_in_progress_flag());
    
    unsigned char s = get_rtc_register(0x00);
    unsigned char m = get_rtc_register(0x02);
    unsigned char h = get_rtc_register(0x04);

    // konwersja
    *second = BCD_TO_BIN(s);
    *minute = BCD_TO_BIN(m);
    *hour   = BCD_TO_BIN(h);
}