//wilmer henao

#ifndef CRCCHECKS_H
#define CRCCHECKS_H

//////CRC copied specifically from the specs Stuff
void make_crc_table(void);
unsigned long update_crc(unsigned long, unsigned char *,
                            int);
unsigned long crc(unsigned char*, int );

#endif
