/**
 * @file disk.h
 * @brief Header file for disk operations in a simple file system implementation.
 *
 * This file defines the interface for interacting with a virtual disk, including
 * opening, closing, reading, and writing operations. It also specifies constants
 * for block size and block count.
 */

#ifndef DISK_H
#define DISK_H

/**
 * @brief Opens a virtual disk file.
 *
 * This function initializes the virtual disk by opening the file specified by the
 * given path. The file represents the disk storage for the file system.
 *
 * @param path The path to the virtual disk file.
 * @return 0 on success, or a negative value on failure.
 */
int disk_open(const char *path);

/**
 * @brief Closes the virtual disk file.
 *
 * This function closes the virtual disk file and releases any resources associated
 * with it.
 */
void disk_close();

/**
 * @brief Reads a block of data from the virtual disk.
 *
 * This function reads data from the specified block number into the provided buffer.
 *
 * @param block_num The block number to read from (0-based index).
 * @param buf A pointer to the buffer where the data will be stored.
 * @return 0 on success, or a negative value on failure.
 */
int disk_read(int block_num, void *buf);

/**
 * @brief Writes a block of data to the virtual disk.
 *
 * This function writes data from the provided buffer to the specified block number.
 *
 * @param block_num The block number to write to (0-based index).
 * @param buf A pointer to the buffer containing the data to be written.
 * @return 0 on success, or a negative value on failure.
 */
int disk_write(int block_num, const void *buf);

/**
 * @def BLOCK_SIZE
 * @brief The size of a single block in bytes.
 *
 * This constant defines the size of each block in the virtual disk.
 */
#define BLOCK_SIZE 1024

/**
 * @def BLOCK_COUNT
 * @brief The total number of blocks in the virtual disk.
 *
 * This constant defines the total number of blocks available in the virtual disk.
 */
#define BLOCK_COUNT 1024

#endif