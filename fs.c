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
        if (find_dir_entry(&inode, parts[i], &entry) != 0) {
            printf("Path component '%s' not found in inode %d\n", parts[i], current_inum);
            return -1;  // Not found
        }

        current_inum = entry.inum;
    }

    if (out_inum) *out_inum = current_inum;
    return 0;
}

// Mkdir function
int mkdir_fs(const char *path) {
    // Step 1: Resolve the parent directory inode
    int parent_inum;
    if (path_to_inode(path, &parent_inum, 1) != 0) {
        fprintf(stderr, "mkdir_fs: Failed to resolve parent directory for path %s\n", path);
        return -1;
    }

    // Step 2: Extract the name of the new directory
    char parts[64][MAX_FILENAME_LEN + 1];
    int count;
    if (split_path(path, parts, &count) != 0 || count == 0) {
        fprintf(stderr, "mkdir_fs: Invalid path %s\n", path);
        return -1;
    }

    char *dirname = parts[count - 1]; // The new directory name

    // Step 3: Read the parent inode
    Inode parent;
    if (read_inode(parent_inum, &parent) != 0) {
        fprintf(stderr, "mkdir_fs: Failed to read parent inode %d\n", parent_inum);
        return -1;
    }

    if (!parent.is_directory) {
        fprintf(stderr, "mkdir_fs: Parent inode %d is not a directory\n", parent_inum);
        return -1;
    }

    // Step 4: Check if the directory already exists
    DirectoryEntry dummy;
    if (find_dir_entry(&parent, dirname, &dummy) == 0) {
        fprintf(stderr, "mkdir_fs: Directory '%s' already exists\n", dirname);
        return -1;
    }

    // Step 5: Allocate new inode for the directory
    int new_inum = allocate_inode();
    if (new_inum < 0) {
        fprintf(stderr, "mkdir_fs: Failed to allocate inode for new directory\n");
        return -1;
    }

    // Step 6: Initialize the new inode as a directory
    Inode new_dir;
    new_dir.is_valid = 1;
    new_dir.is_directory = 1;
    new_dir.size = 0;
    memset(new_dir.direct_blocks, 0, sizeof(new_dir.direct_blocks));

    if (write_inode(new_inum, &new_dir) != 0) {
        fprintf(stderr, "mkdir_fs: Failed to write new directory inode %d\n", new_inum);
        return -1;
    }

    // Step 7: Add directory entry to parent
    char block[BLOCK_SIZE];
    int entry_added = 0;

    for (int i = 0; i < MAX_DIRECT_POINTERS && !entry_added; i++) {
        if (parent.direct_blocks[i] == 0) {
            // Allocate a new block for directory entries
            int new_block = allocate_block();
            if (new_block < 0) {
                fprintf(stderr, "mkdir_fs: Failed to allocate block for directory entries\n");
                return -1;
            }

            parent.direct_blocks[i] = new_block;
            memset(block, 0, BLOCK_SIZE);

            // Fix: SAVE THE UPDATED PARENT INODE IMMEDIATELY
            if (write_inode(parent_inum, &parent) != 0) {
                fprintf(stderr, "mkdir_fs: Failed to save parent inode after block allocation\n");
                return -1;
            }

        } else {
            // Load existing directory entry block
            if (disk_read(parent.direct_blocks[i], block) != 0) {
                fprintf(stderr, "mkdir_fs: Failed to read existing directory block %d\n", parent.direct_blocks[i]);
                return -1;
            }
        }

        DirectoryEntry *entries = (DirectoryEntry *)block;
        int entries_per_block = BLOCK_SIZE / sizeof(DirectoryEntry);

        for (int j = 0; j < entries_per_block; j++) {
            if (entries[j].inum == 0) {
                entries[j].inum = new_inum;
                strncpy(entries[j].name, dirname, MAX_FILENAME_LEN);
                entries[j].name[MAX_FILENAME_LEN] = '\0';

                // Write back updated directory block
                if (disk_write(parent.direct_blocks[i], block) != 0) {
                    fprintf(stderr, "mkdir_fs: Failed to write directory block back to disk\n");
                    return -1;
                }

                parent.size += sizeof(DirectoryEntry);
                if (write_inode(parent_inum, &parent) != 0) {
                    fprintf(stderr, "mkdir_fs: Failed to update parent inode with new size\n");
                    return -1;
                }

                printf("mkdir_fs: Created directory '%s' (inode %d) under parent inode %d\n",
                       dirname, new_inum, parent_inum);

                entry_added = 1;
                break;
            }
        }
    }

    if (!entry_added) {
        fprintf(stderr, "mkdir_fs: No space in parent directory to add new entry '%s'\n", dirname);
        return -1;
    }

    return 0;
}


// create_fs(): creates an empty file at the given absolute path.
// Returns 0 on success, -1 on failure.
int create_fs(const char *path) {
    // First, split path and get the parent inode.
    int parent_inum;
    if (path_to_inode(path, &parent_inum, 1) != 0) {
        fprintf(stderr, "create_fs: Failed to get parent inode for path %s\n", path);
        return -1;
    }
    
    // Split the path into parts to get the filename (last part)
    char parts[64][MAX_FILENAME_LEN + 1];
    int count;
    if (split_path(path, parts, &count) != 0 || count == 0) {
        fprintf(stderr, "create_fs: Failed to split path %s\n", path);
        return -1;
    }
    // The last part is the file name:
    char *filename = parts[count - 1];

    // Read the parent inode
    Inode parent;
    if (read_inode(parent_inum, &parent) != 0) {
        fprintf(stderr, "create_fs: Failed to read parent inode %d\n", parent_inum);
        return -1;
    }
    if (!parent.is_directory) {
        fprintf(stderr, "create_fs: Parent inode %d is not a directory\n", parent_inum);
        return -1;
    }

    // Check if the file already exists in the parent's directory entries.
    DirectoryEntry dummy;
    if (find_dir_entry(&parent, filename, &dummy) == 0) {
        fprintf(stderr, "create_fs: File %s already exists in parent directory\n", filename);
        return -1;
    }

    // Allocate a new inode for the file.
    int new_inum = allocate_inode();
    if (new_inum < 0) {
        fprintf(stderr, "create_fs: Failed to allocate new inode for %s\n", filename);
        return -1;
    }

    // Prepare the new file inode: mark as valid, not a directory, size zero.
    Inode new_file;
    new_file.is_valid = 1;
    new_file.is_directory = 0;  // file, not a directory
    new_file.size = 0;
    memset(new_file.direct_blocks, 0, sizeof(new_file.direct_blocks));

    if (write_inode(new_inum, &new_file) != 0) {
        fprintf(stderr, "create_fs: Failed to write new inode %d\n", new_inum);
        return -1;
    }

    // Now, add an entry for this new file into the parent directory.
    // For simplicity, we search parent's direct_blocks for an available slot.
    int entry_added = 0;
    char block_buf[BLOCK_SIZE];
    for (int i = 0; i < MAX_DIRECT_POINTERS && !entry_added; i++) {
        // If the parent's direct block is not allocated yet, allocate one.
        if (parent.direct_blocks[i] == 0) {
            int blk = allocate_block();
            if (blk < 0) {
                fprintf(stderr, "create_fs: Failed to allocate a data block for directory entries\n");
                return -1;
            }
            parent.direct_blocks[i] = blk;
            memset(block_buf, 0, BLOCK_SIZE);
        } else {
            // Read the existing block into block_buf.
            disk_read(parent.direct_blocks[i], block_buf);
        }

        // How many directory entries fit in one block?
        int entry_count = BLOCK_SIZE / sizeof(DirectoryEntry);
        DirectoryEntry *entries = (DirectoryEntry *)block_buf;

        // Find an empty slot (inum == 0 means free)
        for (int j = 0; j < entry_count; j++) {
            if (entries[j].inum == 0) {
                entries[j].inum = new_inum;
                // Copy the filename (ensure null-termination)
                strncpy(entries[j].name, filename, MAX_FILENAME_LEN);
                entries[j].name[MAX_FILENAME_LEN] = '\0';
                // Write the block back to disk
                if (disk_write(parent.direct_blocks[i], block_buf) != 0) {
                    fprintf(stderr, "create_fs: Failed to update directory entry block\n");
                    return -1;
                }
                // Update parent's size (optional: increment by size of entry)
                parent.size += sizeof(DirectoryEntry);
                // Write the updated parent inode back
                if (write_inode(parent_inum, &parent) != 0) {
                    fprintf(stderr, "create_fs: Failed to update parent inode\n");
                    return -1;
                }
                entry_added = 1;
                break;
            }
        }
    }

    if (!entry_added) {
        fprintf(stderr, "create_fs: No free directory entry slot for file %s\n", filename);
        return -1;
    }

    return 0;
}

// write_fs(): writes data to the file at the given absolute path.
// 'data' is a pointer to the bytes, and 'size' is the number of bytes to write.
// Returns number of bytes written on success, -1 on failure.
int write_fs(const char *path, const void *data, size_t size) {
    // Limit to maximum file size (4 blocks)
    if (size > MAX_DIRECT_POINTERS * BLOCK_SIZE) {
        fprintf(stderr, "write_fs: File size too large (max is %d bytes)\n", MAX_DIRECT_POINTERS * BLOCK_SIZE);
        return -1;
    }

    // Get the file's inode number
    int file_inum;
    if (path_to_inode(path, &file_inum, 0) != 0) {
        fprintf(stderr, "write_fs: File %s not found\n", path);
        return -1;
    }

    Inode file;
    if (read_inode(file_inum, &file) != 0) {
        fprintf(stderr, "write_fs: Failed to read inode for file %s\n", path);
        return -1;
    }

    // For simplicity, free any pre-allocated data blocks before writing new data.
    // (In a more complete FS, you would handle writing over existing data.)
    for (int i = 0; i < MAX_DIRECT_POINTERS; i++) {
        if (file.direct_blocks[i] != 0) {
            free_block(file.direct_blocks[i]);
            file.direct_blocks[i] = 0;
        }
    }

    size_t bytes_left = size;
    const char *data_ptr = data;
    int block_index = 0;

    // Write data into as many blocks as needed.
    while (bytes_left > 0 && block_index < MAX_DIRECT_POINTERS) {
        int blk = allocate_block();
        if (blk < 0) {
            fprintf(stderr, "write_fs: No free data block available\n");
            return -1;
        }

        file.direct_blocks[block_index] = blk;
        char block_data[BLOCK_SIZE];
        memset(block_data, 0, BLOCK_SIZE);

        // Determine how many bytes to write in this block.
        size_t to_write = (bytes_left > BLOCK_SIZE) ? BLOCK_SIZE : bytes_left;
        memcpy(block_data, data_ptr, to_write);
        if (disk_write(blk, block_data) != 0) {
            fprintf(stderr, "write_fs: Error writing block %d\n", blk);
            return -1;
        }

        data_ptr += to_write;
        bytes_left -= to_write;
        block_index++;
    }

    file.size = size;
    if (write_inode(file_inum, &file) != 0) {
        fprintf(stderr, "write_fs: Failed to update inode for file %s\n", path);
        return -1;
    }

    return (int)size;
}

// read_fs(): reads data from the file at the given path into the provided buffer.
// Reads up to 'size' bytes; the file's actual size may be less.
// Returns number of bytes read, or -1 on failure.
int read_fs(const char *path, void *buffer, size_t size) {
    // Get the file's inode number
    int file_inum;
    if (path_to_inode(path, &file_inum, 0) != 0) {
        fprintf(stderr, "read_fs: File %s not found\n", path);
        return -1;
    }

    Inode file;
    if (read_inode(file_inum, &file) != 0) {
        fprintf(stderr, "read_fs: Failed to read inode for file %s\n", path);
        return -1;
    }

    // Only read up to the file's size
    if (size > file.size)
        size = file.size;

    size_t bytes_left = size;
    char *buf_ptr = buffer;
    int total_read = 0;
    int block_index = 0;

    while (bytes_left > 0 && block_index < MAX_DIRECT_POINTERS) {
        if (file.direct_blocks[block_index] == 0) break;  // No more blocks allocated
        char block_data[BLOCK_SIZE];
        if (disk_read(file.direct_blocks[block_index], block_data) != 0) {
            fprintf(stderr, "read_fs: Error reading block %d\n", file.direct_blocks[block_index]);
            return -1;
        }
        size_t to_read = (bytes_left > BLOCK_SIZE) ? BLOCK_SIZE : bytes_left;
        memcpy(buf_ptr, block_data, to_read);
        buf_ptr += to_read;
        bytes_left -= to_read;
        total_read += to_read;
        block_index++;
    }

    return total_read;
}

int delete_fs(const char *path) {
    // Step 1: Parse and validate the path
    char parts[64][MAX_FILENAME_LEN + 1];
    int count;
    if (split_path(path, parts, &count) != 0 || count == 0) {
        fprintf(stderr, "delete_fs: Invalid or empty path\n");
        return -1;
    }

    // Step 2: Resolve parent directory
    int parent_inum;
    if (path_to_inode(path, &parent_inum, 1) != 0) {
        fprintf(stderr, "delete_fs: Failed to resolve parent for %s\n", path);
        return -1;
    }

    // Step 3: Read parent inode
    Inode parent;
    if (read_inode(parent_inum, &parent) != 0 || !parent.is_valid || !parent.is_directory) {
        fprintf(stderr, "delete_fs: Invalid parent inode %d\n", parent_inum);
        return -1;
    }

    // Step 4: Get the name of the file/directory to delete
    char *target_name = parts[count - 1];

    // Step 5: Find the directory entry inside the parent
    DirectoryEntry target_entry;
    if (find_dir_entry(&parent, target_name, &target_entry) != 0) {
        fprintf(stderr, "delete_fs: Entry '%s' not found in parent\n", target_name);
        return -1;
    }

    int target_inum = target_entry.inum;

    // Step 6: Read the inode of the target
    Inode target;
    if (read_inode(target_inum, &target) != 0 || !target.is_valid) {
        fprintf(stderr, "delete_fs: Invalid or unreadable inode %d\n", target_inum);
        return -1;
    }

    // Step 7: If target is a directory, ensure it's empty
    if (target.is_directory) {
        char block[BLOCK_SIZE];
        for (int i = 0; i < MAX_DIRECT_POINTERS; i++) {
            if (target.direct_blocks[i] == 0) continue;
            disk_read(target.direct_blocks[i], block);
            DirectoryEntry *entries = (DirectoryEntry *)block;
            int num_entries = BLOCK_SIZE / sizeof(DirectoryEntry);
            for (int j = 0; j < num_entries; j++) {
                if (entries[j].inum != 0) {
                    fprintf(stderr, "delete_fs: Directory '%s' is not empty\n", target_name);
                    return -1;
                }
            }
        }
    }

    // Step 8: Free all data blocks of the target
    for (int i = 0; i < MAX_DIRECT_POINTERS; i++) {
        if (target.direct_blocks[i] != 0) {
            free_block(target.direct_blocks[i]);
            target.direct_blocks[i] = 0;
        }
    }

    // Step 9: Invalidate the inode
    target.is_valid = 0;
    write_inode(target_inum, &target);

    // Step 10: Remove the directory entry from the parent
    char block[BLOCK_SIZE];
    for (int i = 0; i < MAX_DIRECT_POINTERS; i++) {
        if (parent.direct_blocks[i] == 0) continue;
        disk_read(parent.direct_blocks[i], block);
        DirectoryEntry *entries = (DirectoryEntry *)block;
        int entry_count = BLOCK_SIZE / sizeof(DirectoryEntry);
        for (int j = 0; j < entry_count; j++) {
            if (entries[j].inum == target_inum && strcmp(entries[j].name, target_name) == 0) {
                entries[j].inum = 0;
                entries[j].name[0] = '\0';  // Clear name
                disk_write(parent.direct_blocks[i], block);
                parent.size -= sizeof(DirectoryEntry);
                write_inode(parent_inum, &parent);
                return 0;
            }
        }
    }

    fprintf(stderr, "delete_fs: Could not remove entry from parent directory\n");
    return -1;
}
