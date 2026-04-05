#ifndef RTC_H
#define RTC_H

unsigned char get_update_in_progress_flag();
unsigned char get_rtc_register(int reg);
void read_rtc(int* second, int* minute, int* hour);

#endif