#ifndef DISK_H
#define DISK_H

int disk_open(const char *path);
void disk_close();
int disk_read(int block_num, void *buf);
int disk_write(int block_num, const void *buf);

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 1024

#endif
