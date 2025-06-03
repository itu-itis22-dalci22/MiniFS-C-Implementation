#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024  // Size of each block in bytes
#define BLOCK_COUNT 1024 // Total number of blocks on the disk

static FILE *disk_file = NULL; // File pointer for the simulated disk

// Opens the disk file at the specified path in read/write binary mode.
// Returns 0 on success, -1 on failure.
int disk_open(const char *path) {  
    disk_file = fopen(path, "r+b"); // Open file in read/write binary mode
    return disk_file != NULL ? 0 : -1; // Check if the file was successfully opened
}

// Closes the disk file if it is open.
void disk_close() {
    if (disk_file) fclose(disk_file); // Close the file if it is open
}

// Reads a block of data from the disk into the provided buffer.
// block_num: The block number to read (0-based index).
// buf: Pointer to the buffer where the data will be stored.
// Returns 0 on success, -1 on failure.
int disk_read(int block_num, void *buf) {
    if (block_num < 0 || block_num >= BLOCK_COUNT) return -1; // Validate block number
    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET); // Move file pointer to the start of the block
    return fread(buf, 1, BLOCK_SIZE, disk_file) == BLOCK_SIZE ? 0 : -1; // Read the block and check for success
}

// Writes a block of data to the disk from the provided buffer.
// block_num: The block number to write (0-based index).
// buf: Pointer to the buffer containing the data to write.
// Returns 0 on success, -1 on failure.
int disk_write(int block_num, const void *buf) {
    if (block_num < 0 || block_num >= BLOCK_COUNT) return -1; // Validate block number
    fseek(disk_file, block_num * BLOCK_SIZE, SEEK_SET); // Move file pointer to the start of the block
    return fwrite(buf, 1, BLOCK_SIZE, disk_file) == BLOCK_SIZE ? 0 : -1; // Write the block and check for success
}
