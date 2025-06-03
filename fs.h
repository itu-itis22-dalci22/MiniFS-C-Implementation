/**
 * @file fs.h
 * @brief Header file for a simple file system implementation in C.
 *
 * This file defines the constants, structures, and function prototypes
 * for a basic file system. The file system supports operations such as
 * creating files, reading/writing data, managing directories, and handling
 * inodes and block allocation.
 */

#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Size of a single block in bytes.
 */
#define BLOCK_SIZE 1024

/**
 * @brief Total number of blocks in the file system.
 */
#define BLOCK_COUNT 1024

/**
 * @brief Number of blocks reserved for storing inodes.
 */
#define INODE_BLOCKS 9

/**
 * @brief Starting block index for inodes.
 */
#define INODE_START 2

/**
 * @brief Total number of inodes in the file system.
 */
#define INODE_COUNT 128

/**
 * @brief Starting block index for data blocks.
 */
#define DATA_START (INODE_START + INODE_BLOCKS)

/**
 * @brief Magic number used to identify the file system.
 */
#define MAGIC_NUMBER 0xf00dbeef

/**
 * @brief Block index for the bitmap used to track free/used blocks.
 */
#define BITMAP_BLOCK 1

/**
 * @brief Starting block index for data blocks.
 */
#define DATA_BLOCK_START 11

/**
 * @brief Total number of data blocks available in the file system.
 */
#define DATA_BLOCK_COUNT (BLOCK_COUNT - DATA_BLOCK_START)

/**
 * @brief Maximum number of direct block pointers in an inode.
 */
#define MAX_DIRECT_POINTERS 4

/**
 * @brief Maximum length of a filename (excluding null terminator).
 */
#define MAX_FILENAME_LEN 27

/**
 * @struct DirectoryEntry
 * @brief Represents a directory entry in the file system.
 *
 * @param inum Inode number associated with the directory entry.
 * @param name Name of the file or directory (null-terminated string).
 */
typedef struct {
    uint32_t inum;
    char name[MAX_FILENAME_LEN + 1];  // +1 for null terminator
} DirectoryEntry;

/**
 * @struct SuperBlock
 * @brief Represents the superblock of the file system.
 *
 * @param magic Magic number identifying the file system.
 * @param block_size Size of each block in bytes.
 * @param fs_size_blocks Total size of the file system in blocks.
 * @param inode_start Starting block index for inodes.
 * @param inode_count Total number of inodes in the file system.
 * @param data_start Starting block index for data blocks.
 */
typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t fs_size_blocks;
    uint32_t inode_start;
    uint32_t inode_count;
    uint32_t data_start;
} SuperBlock;

/**
 * @struct Inode
 * @brief Represents an inode in the file system.
 *
 * @param size Size of the file or directory in bytes.
 * @param direct_blocks Array of direct block pointers.
 * @param is_valid Indicates whether the inode is valid (1 for valid, 0 for invalid).
 * @param is_directory Indicates whether the inode represents a directory (1 for directory, 0 for file).
 * @param owner_id ID of the owner of the inode.
 */
typedef struct {
    uint32_t size;
    uint32_t direct_blocks[MAX_DIRECT_POINTERS];
    uint8_t  is_valid;
    uint8_t  is_directory;
    int owner_id;
} Inode;

// --- Function declarations ---

/**
 * @brief Creates a new file system on the specified path.
 *
 * @param path Path to the file system image.
 * @return 0 on success, -1 on failure.
 */
int mkfs_fs(const char *path);

// File system initialization and cleanup

/**
 * @brief Initializes the file system from the specified disk image.
 *
 * @param disk_path Path to the disk image.
 * @return 0 on success, -1 on failure.
 */
int init_fs(const char* disk_path);

/**
 * @brief Cleans up resources used by the file system.
 */
void cleanup_fs();

// --- Bitmap function declarations ---

/**
 * @brief Loads the block allocation bitmap from disk.
 */
void load_bitmap();

/**
 * @brief Saves the block allocation bitmap to disk.
 */
void save_bitmap();

/**
 * @brief Allocates a free block and returns its block number.
 *
 * @return Block number on success, -1 on failure.
 */
int allocate_block();

/**
 * @brief Frees the specified block, marking it as available.
 *
 * @param block_num Block number to free.
 */
void free_block(int block_num);

/**
 * @brief Checks if the specified block is free.
 *
 * @param block_num Block number to check.
 * @return 1 if the block is free, 0 if it is used.
 */
int is_block_free(int block_num);

/**
 * @brief Marks the specified block as used.
 *
 * @param block_num Block number to mark as used.
 */
void mark_block_used(int block_num);

/**
 * @brief Marks the specified block as free.
 *
 * @param block_num Block number to mark as free.
 */
void mark_block_free(int block_num);

// --- Inode Table function declarations ---

/**
 * @brief Reads an inode from the inode table.
 *
 * @param inode_num Inode number to read.
 * @param inode Pointer to the inode structure to populate.
 * @return 0 on success, -1 on failure.
 */
int read_inode(int inode_num, Inode *inode);

/**
 * @brief Writes an inode to the inode table.
 *
 * @param inode_num Inode number to write.
 * @param inode Pointer to the inode structure to write.
 * @return 0 on success, -1 on failure.
 */
int write_inode(int inode_num, Inode *inode);

/**
 * @brief Allocates a new inode and returns its inode number.
 *
 * @return Inode number on success, -1 on failure.
 */
int allocate_inode();

/**
 * @brief Frees the specified inode, marking it as available.
 *
 * @param inode_num Inode number to free.
 */
void free_inode(int inode_num);

// Path Resolution function declarations

/**
 * @brief Splits a file path into its components.
 *
 * @param path File path to split.
 * @param dir Array to store the path components.
 * @param count Pointer to store the number of components.
 * @return 0 on success, -1 on failure.
 */
int split_path(const char *path, char dir[][MAX_FILENAME_LEN + 1], int *count);

/**
 * @brief Finds a directory entry by name within a directory inode.
 *
 * @param dir_inode Pointer to the directory inode.
 * @param name Name of the directory entry to find.
 * @param out_entry Pointer to store the found directory entry.
 * @return 0 on success, -1 on failure.
 */
int find_dir_entry(Inode *dir_inode, const char *name, DirectoryEntry *out_entry);

/**
 * @brief Resolves a file path to its inode number.
 *
 * @param path File path to resolve.
 * @param out_inum Pointer to store the resolved inode number.
 * @param want_parent If non-zero, resolves to the parent directory's inode.
 * @return 0 on success, -1 on failure.
 */
int path_to_inode(const char *path, int *out_inum, int want_parent);

// Directory operations

/**
 * @brief Creates a new directory at the specified path.
 *
 * @param path Path to the new directory.
 * @return 0 on success, -1 on failure.
 */
int mkdir_fs(const char *path);

// File operations

/**
 * @brief Creates a new file at the specified path.
 *
 * @param path Path to the new file.
 * @return 0 on success, -1 on failure.
 */
int create_fs(const char *path);

/**
 * @brief Writes data to a file at the specified path.
 *
 * @param path Path to the file.
 * @param data Pointer to the data to write.
 * @param size Size of the data to write in bytes.
 * @return Number of bytes written on success, -1 on failure.
 */
int write_fs(const char *path, const void *data, size_t size);

/**
 * @brief Reads data from a file at the specified path.
 *
 * @param path Path to the file.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size Size of the buffer in bytes.
 * @return Number of bytes read on success, -1 on failure.
 */
int read_fs(const char *path, void *buffer, size_t size);

/**
 * @brief Deletes a file at the specified path.
 *
 * @param path Path to the file.
 * @return 0 on success, -1 on failure.
 */
int delete_fs(const char *path);

/**
 * @brief Removes a directory at the specified path.
 *
 * @param path Path to the directory.
 * @return 0 on success, -1 on failure.
 */
int rmdir_fs(const char *path);

/**
 * @brief Lists the contents of a directory at the specified path.
 *
 * @param path Path to the directory.
 * @param entries Array to store the directory entries.
 * @param max_entries Maximum number of entries to store.
 * @return Number of entries listed on success, -1 on failure.
 */
int ls_fs(const char *path, DirectoryEntry *entries, int max_entries);

#endif