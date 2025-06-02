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
#define MAX_DIRECT_POINTERS 4

#define MAX_FILENAME_LEN 27

typedef struct {
    uint32_t inum;
    char name[MAX_FILENAME_LEN + 1];  // +1 for null terminator
} DirectoryEntry;

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t fs_size_blocks;
    uint32_t inode_start;
    uint32_t inode_count;
    uint32_t data_start;
} SuperBlock;

typedef struct {
    uint32_t size;
    uint32_t direct_blocks[MAX_DIRECT_POINTERS];
    uint8_t  is_valid;
    uint8_t  is_directory;
    uint16_t reserved; // manually align to 4 bytes
} Inode;

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

// --- Inode Table function declarations ---
int read_inode(int inode_num, Inode *inode);
int write_inode(int inode_num, Inode *inode);
int allocate_inode();
void free_inode(int inode_num);

// Path Resolution function declarations

int split_path(const char *path, char dir[][MAX_FILENAME_LEN + 1], int *count);
int find_dir_entry(Inode *dir_inode, const char *name, DirectoryEntry *out_entry);
int path_to_inode(const char *path, int *out_inum, int want_parent);

// Directory operations
int mkdir_fs(const char *path);

#endif
