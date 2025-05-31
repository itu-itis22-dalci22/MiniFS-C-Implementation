#include "fs.h"
#include "disk.h"
#include <string.h>
#include <stdio.h>

int mkfs_fs(const char *disk_path) {
    FILE *f = fopen(disk_path, "wb");
    if (!f) return -1;

    char zero[BLOCK_SIZE] = {0};
    for (int i = 0; i < BLOCK_COUNT; i++) fwrite(zero, 1, BLOCK_SIZE, f);
    fclose(f);

    if (disk_open(disk_path) != 0) return -1;

    // Create superblock
    SuperBlock sb;
    sb.magic = MAGIC_NUMBER;
    sb.block_size = BLOCK_SIZE;
    sb.fs_size_blocks = BLOCK_COUNT;
    sb.inode_start = INODE_START;
    sb.inode_count = INODE_COUNT;
    sb.data_start = DATA_START;

    // Write superblock to block 0
    disk_write(0, &sb);

    // Zero bitmap (block 1)
    char bitmap_block[BLOCK_SIZE] = {0};
    disk_write(1, bitmap_block);

    // Zero inode table blocks (blocks 2–10)
    for (int i = 0; i < INODE_BLOCKS; i++) {
        disk_write(INODE_START + i, zero);
    }

    // Mark root inode (we'll define inode structure next step)
    // Reserve inode 0 for "/"

    disk_close();
    return 0;
}
