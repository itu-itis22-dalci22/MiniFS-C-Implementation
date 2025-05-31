#include "disk.h"
#include <string.h>
#include <stdio.h>

int main() {
    if (disk_open("disk.img") != 0) {
        printf("Creating new disk image...\n");
        FILE *f = fopen("disk.img", "wb");
        char zero[BLOCK_SIZE] = {0};
        for (int i = 0; i < BLOCK_COUNT; i++) fwrite(zero, 1, BLOCK_SIZE, f);
        fclose(f);
        disk_open("disk.img");
    }

    char write_buf[BLOCK_SIZE] = "Hello, FS!";
    char read_buf[BLOCK_SIZE] = {0};

    disk_write(5, write_buf);
    disk_read(5, read_buf);

    printf("Read from disk: %s\n", read_buf);
    disk_close();
    return 0;
}
