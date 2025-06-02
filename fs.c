#include "fs.h"
#include "disk.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

// Inode operations

int read_inode(int inum, Inode *inode) {
    if (inum < 0 || inum >= INODE_COUNT) return -1;

    int inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    int block = INODE_START + inum / inodes_per_block;
    int offset = inum % inodes_per_block;

    if (block < INODE_START || block >= INODE_START + INODE_BLOCKS) {
        fprintf(stderr, "read_inode: Block %d is outside inode table!\n", block);
        return -1;
    }

    Inode *inodes = malloc(BLOCK_SIZE);
    if (!inodes) {
        fprintf(stderr, "read_inode: Memory allocation failed!\n");
        return -1;
    }

    if (disk_read(block, inodes) != 0) {
        fprintf(stderr, "Failed to read inode block %d\n", block);
        free(inodes);
        return -1;
    }

    *inode = inodes[offset];

    free(inodes);
    return 0;
}

int write_inode(int inum, Inode *inode) {
    if (inum < 0 || inum >= INODE_COUNT) return -1;

    int inodes_per_block = BLOCK_SIZE / sizeof(Inode);
    int block = INODE_START + inum / inodes_per_block;
    int offset = inum % inodes_per_block;

    printf("Writing inode %d to block %d, offset %d\n", inum, block, offset);

    if (block < INODE_START || block >= INODE_START + INODE_BLOCKS) {
        fprintf(stderr, "Block %d is outside inode table!\n", block);
        return -1;
    }

    Inode *inodes = malloc(BLOCK_SIZE);
    if (!inodes) {
        fprintf(stderr, "Memory allocation failed!\n");
        return -1;
    }

    if (disk_read(block, inodes) != 0) {
        fprintf(stderr, "Failed to read block %d\n", block);
        free(inodes);
        return -1;
    }

    inodes[offset] = *inode;

    if (disk_write(block, inodes) != 0) {
        fprintf(stderr, "Failed to write block %d\n", block);
        free(inodes);
        return -1;
    }

    free(inodes);
    printf("Inode %d written successfully.\n", inum);
    return 0;
}

int allocate_inode() {
    Inode inode;

    for (int i = 0; i < INODE_COUNT; i++) {
        if (read_inode(i, &inode) != 0) continue;
        if (!inode.is_valid) {
            inode.is_valid = 1;
            inode.is_directory = 0;
            inode.size = 0;
            memset(inode.direct_blocks, 0, sizeof(inode.direct_blocks));
            write_inode(i, &inode);
            return i;
        }
    }
    return -1;  // No free inode
}
 
void free_inode(int inum) {
    Inode inode;
    if (read_inode(inum, &inode) != 0) return;
    inode.is_valid = 0;
    write_inode(inum, &inode);
}

// Create a new filesystem on the disk

int mkfs_fs(const char *disk_path) {
    // 1. Create and zero-fill a new disk image
    FILE *f = fopen(disk_path, "wb");
    if (!f) return -1;

    char zero[BLOCK_SIZE] = {0};
    for (int i = 0; i < BLOCK_COUNT; i++) {
        fwrite(zero, 1, BLOCK_SIZE, f);
    }
    fclose(f);

    // 2. Open the disk using your disk I/O abstraction
    if (disk_open(disk_path) != 0) return -1;

    // 3. Create and write the superblock (block 0)
    SuperBlock sb;
    sb.magic = MAGIC_NUMBER;
    sb.block_size = BLOCK_SIZE;
    sb.fs_size_blocks = BLOCK_COUNT;
    sb.inode_start = INODE_START;
    sb.inode_count = INODE_COUNT;
    sb.data_start = DATA_START;

    disk_write(0, &sb); // Block 0

    // 4. Zero bitmap (block 1)
    char bitmap_block[BLOCK_SIZE] = {0};
    disk_write(1, bitmap_block); // Block 1

    // 5. Zero inode table blocks (blocks 2–10)
    for (int i = 0; i < INODE_BLOCKS; i++) {
        disk_write(INODE_START + i, zero); // Blocks 2 to 10
    }

    // 6. Initialize inode 0 as root directory "/"
    Inode root;
    root.is_valid = 1;
    root.is_directory = 1;
    root.size = 0;
    memset(root.direct_blocks, 0, sizeof(root.direct_blocks));

    write_inode(0, &root);  // Save root inode to disk

    disk_close();
    return 0;
}
 
int split_path(const char *path, char parts[][MAX_FILENAME_LEN + 1], int *count) {
    if (!path || path[0] != '/' || !parts || !count) return -1;  // Validate all input

    *count = 0;
    const char *p = path + 1; // skip initial '/'

    while (p && *p && *count < 64) {
        const char *slash = strchr(p, '/');  // Find next '/'
        int len = slash ? (slash - p) : strlen(p);  // Length of this part

        if (len > MAX_FILENAME_LEN || len == 0) return -1;  // Check name length

        strncpy(parts[*count], p, len);  // Copy the part
        parts[*count][len] = '\0';       // Null-terminate
        (*count)++;

        p = slash ? slash + 1 : NULL;    // Move to next component
    }

    return 0;
}


int find_dir_entry(Inode *dir_inode, const char *name, DirectoryEntry *out_entry) {
    char block[BLOCK_SIZE];

    for (int i = 0; i < MAX_DIRECT_POINTERS; i++) {
        if (dir_inode->direct_blocks[i] == 0) continue;

        disk_read(dir_inode->direct_blocks[i], block);

        int count = BLOCK_SIZE / sizeof(DirectoryEntry);
        DirectoryEntry *entries = (DirectoryEntry *)block;

        for (int j = 0; j < count; j++) {
            if (entries[j].inum != 0 && strcmp(entries[j].name, name) == 0) {
                if (out_entry) *out_entry = entries[j];
                return 0;
            }
        }
    }
    return -1; // not found
}

int path_to_inode(const char *path, int *out_inum, int want_parent) {
    char parts[64][MAX_FILENAME_LEN + 1];
    int count=0;

    if (split_path(path, parts, &count) != 0) return -1;

    int current_inum = 0;  // Start from root
    Inode inode;

    for (int i = 0; i < count - want_parent; i++) {
        if (read_inode(current_inum, &inode) != 0) return -1;
        if (!inode.is_valid || !inode.is_directory) return -1;

        DirectoryEntry entry;
        if (find_dir_entry(&inode, parts[i], &entry) != 0) return -1;

        current_inum = entry.inum;
    }

    if (out_inum) *out_inum = current_inum;
    return 0;
}

// Mkdir function

int mkdir_fs(const char *path) {
    int parent_inum;
    if (path_to_inode(path, &parent_inum, 1) != 0) return -1;

    char parts[64][MAX_FILENAME_LEN + 1];
    int count;
    if (split_path(path, parts, &count) != 0) return -1;

    Inode parent;
    if (read_inode(parent_inum, &parent) != 0) return -1;

    // Check if already exists
    DirectoryEntry dummy;
    if (find_dir_entry(&parent, parts[count - 1], &dummy) == 0) return -1;

    // Allocate new inode
    int new_inum = allocate_inode();
    if (new_inum < 0) return -1;

    Inode new_dir = {
        .is_valid = 1,
        .is_directory = 1,
        .size = 0
    };
    memset(new_dir.direct_blocks, 0, sizeof(new_dir.direct_blocks));
    write_inode(new_inum, &new_dir);

    // Add to parent's directory block
    char block[BLOCK_SIZE];
    DirectoryEntry *entries = (DirectoryEntry *)block;
    int found_slot = 0;

    for (int i = 0; i < MAX_DIRECT_POINTERS && !found_slot; i++) {
        if (parent.direct_blocks[i] == 0) {
            int new_block = allocate_block();
            if (new_block < 0) return -1;
            parent.direct_blocks[i] = new_block;
            memset(block, 0, BLOCK_SIZE);
        } else {
            disk_read(parent.direct_blocks[i], block);
        }

        int count = BLOCK_SIZE / sizeof(DirectoryEntry);
        for (int j = 0; j < count; j++) {
            if (entries[j].inum == 0) {
                entries[j].inum = new_inum;
                strncpy(entries[j].name, parts[count - 1], MAX_FILENAME_LEN);
                entries[j].name[MAX_FILENAME_LEN] = '\0';

                disk_write(parent.direct_blocks[i], block);
                parent.size += sizeof(DirectoryEntry);
                write_inode(parent_inum, &parent);
                found_slot = 1;
                break;
            }
        }
    }

    return found_slot ? 0 : -1;
}
