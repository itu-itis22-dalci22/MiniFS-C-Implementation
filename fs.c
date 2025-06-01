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

// Global buffer for bitmap (loaded once)
static uint8_t bitmap[BLOCK_SIZE];

// Load bitmap from disk
void load_bitmap() {
    disk_read(BITMAP_BLOCK, bitmap);
}

// Save bitmap to disk
void save_bitmap() {
    disk_write(BITMAP_BLOCK, bitmap);
}

// Mark a block as used
void mark_block_used(int block_num) {
    int rel = block_num - DATA_BLOCK_START;
    bitmap[rel / 8] |= (1 << (rel % 8));
}

// Mark a block as free
void mark_block_free(int block_num) {
    int rel = block_num - DATA_BLOCK_START;
    bitmap[rel / 8] &= ~(1 << (rel % 8));
}

// Check if block is free
int is_block_free(int block_num) {
    int rel = block_num - DATA_BLOCK_START;
    return !(bitmap[rel / 8] & (1 << (rel % 8)));
}

// Allocate a free block and return its number, or -1 if full
int allocate_block() {
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        int block_num = i + DATA_BLOCK_START;
        if (is_block_free(block_num)) {
            mark_block_used(block_num);
            save_bitmap();
            return block_num;
        }
    }
    return -1; // No free block found
}

// Free a block
void free_block(int block_num) {
    mark_block_free(block_num);
    save_bitmap();
}