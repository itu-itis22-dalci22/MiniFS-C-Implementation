#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 1024

static FILE *disk_file = NULL;

int disk_open(const char *path) {
    disk_file = fopen(path, "r+b");
    return disk_file != NULL ? 0 : -1;
}

void disk_close() {
    if (disk_file) fclose(disk_file);
}

int disk_read(int block_num, void *buf) {
    if (block_num < 0 || block_num >= BLOCK_COUNT) return -1;
    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET);
    return fread(buf, 1, BLOCK_SIZE, disk_file) == BLOCK_SIZE ? 0 : -1;
}

int disk_write(int block_num, const void *buf) {
    if (block_num < 0 || block_num >= BLOCK_COUNT) return -1;
    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET);
    return fwrite(buf, 1, BLOCK_SIZE, disk_file) == BLOCK_SIZE ? 0 : -1;
}
