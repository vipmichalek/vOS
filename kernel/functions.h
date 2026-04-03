#ifndef FUNCTIONS_H
#define FUNCTIONS_H


int rand();
int strcmp(char* s1, char* s2);
void itoa(unsigned int n, char* str);
void itoa_hex(unsigned int n, char* str);
void* kmalloc(unsigned int size);
char scancode_to_char(unsigned char scancode);
void hex_to_str(unsigned char n, char* out);
void byte_to_hex(unsigned char n, char* out);
#endif