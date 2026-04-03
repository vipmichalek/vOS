#ifndef ATA_H
#define ATA_H
#include "io.h"

void ata_identify(int* cy);
void ata_read_sector(unsigned int lba, unsigned short* buffer);
void ata_write_sector(unsigned int lba, unsigned short* buffer);

#endif