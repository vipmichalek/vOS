#ifndef TIMER_H
#define TIMER_H

void timer_install(unsigned int frequency);
void timer_handler_c();
unsigned long long int get_timer_tick();

#endif