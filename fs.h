#ifndef FS_H
#define FS_H

#include <stdint.h>

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 1024
#define INODE_BLOCKS 9
#define INODE_START 2
#define INODE_COUNT 128
#define DATA_START (INODE_START + INODE_BLOCKS)
#define MAGIC_NUMBER 0xf00dbeef
#define BITMAP_BLOCK 1
#define DATA_BLOCK_START 11
#define DATA_BLOCK_COUNT (BLOCK_COUNT - DATA_BLOCK_START)



typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t fs_size_blocks;
    uint32_t inode_start;
    uint32_t inode_count;
    uint32_t data_start;
} SuperBlock;

// --- Function declarations ---
int mkfs_fs(const char *path);

// --- Bitmap function declarations ---
void load_bitmap();
void save_bitmap();
int allocate_block();
void free_block(int block_num);
int is_block_free(int block_num);
void mark_block_used(int block_num);
void mark_block_free(int block_num);



#endif
