#include "fs.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Prints usage instructions for the program, including available commands and their arguments.
void print_usage(const char *program_name) {
    printf("Usage: %s <command> [arguments]\n", program_name);
    printf("Commands:\n");
    printf("  mkfs                     - Format the disk\n");
    printf("  mkdir_fs <path>          - Create a directory\n");
    printf("  create_fs <path>         - Create a file\n");
    printf("  write_fs <path> <data>   - Write data to a file\n");
    printf("  read_fs <path>           - Read data from a file\n");
    printf("  ls_fs <path>             - List directory contents\n");
    printf("  delete_fs <path>         - Delete a file\n");
    printf("  rmdir_fs <path>          - Remove a directory\n");
}

// Command to format the disk and initialize the filesystem.
int cmd_mkfs() {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Calls the filesystem formatting function and checks for success.
    if (mkfs_fs(disk_name) != 0) {
        printf("Failed to format disk.\n");
        return 1; // Return error code if formatting fails.
    }
    printf("Disk formatted successfully.\n");
    return 0; // Return success code.
}

// Command to create a directory in the filesystem.
int cmd_mkdir_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the directory creation function and checks for success.
    if (mkdir_fs(path) == 0) {
        printf("Directory %s created successfully.\n", path);
    } else {
        printf("Failed to create directory %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to create a file in the filesystem.
int cmd_create_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the file creation function and checks for success.
    if (create_fs(path) == 0) {
        printf("File %s created successfully.\n", path);
    } else {
        printf("Failed to create file %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to write data to a file in the filesystem.
int cmd_write_fs(const char *path, const char *data) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the file writing function and checks for success.
    if (write_fs(path, data, strlen(data)) >= 0) {
        printf("Wrote content to %s.\n", path);
    } else {
        printf("Failed to write to file %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to read data from a file in the filesystem.
int cmd_read_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    char read_buffer[1024] = {0}; // Buffer to store the read data.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the file reading function and checks for success.
    int bytes_read = read_fs(path, read_buffer, sizeof(read_buffer));
    if (bytes_read >= 0) {
        read_buffer[bytes_read] = '\0'; // Null-terminate the read data.
        printf("Read %d bytes from %s: \"%s\"\n", bytes_read, path, read_buffer);
    } else {
        printf("Failed to read from file %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to list the contents of a directory in the filesystem.
int cmd_ls_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    DirectoryEntry entries[10]; // Array to store directory entries.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the directory listing function and checks for success.
    int num_entries = ls_fs(path, entries, 10);
    if (num_entries >= 0) {
        printf("Contents of %s:\n", path);
        for (int i = 0; i < num_entries; i++) {
            printf(" - %s (inode: %u)\n", entries[i].name, entries[i].inum);
        }
    } else {
        printf("Failed to list contents of directory %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to delete a file in the filesystem.
int cmd_delete_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the file deletion function and checks for success.
    if (delete_fs(path) == 0) {
        printf("Deleted file %s successfully.\n", path);
    } else {
        printf("Failed to delete file %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Command to remove a directory in the filesystem.
int cmd_rmdir_fs(const char *path) {
    const char *disk_name = "disk.img"; // Name of the disk image file.
    
    // Initializes the filesystem before performing operations.
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1; // Return error code if initialization fails.
    }
    
    int result = 0; // Variable to store the result of the operation.
    // Calls the directory removal function and checks for success.
    if (rmdir_fs(path) == 0) {
        printf("Removed directory %s successfully.\n", path);
    } else {
        printf("Failed to remove directory %s.\n", path);
        result = 1; // Update result to indicate failure.
    }
    
    cleanup_fs(); // Cleans up resources after the operation.
    return result; // Return the result of the operation.
}

// Main function to parse user input and execute the corresponding command.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]); // Print usage instructions if no command is provided.
        return 1; // Return error code.
    }
    
    const char *command = argv[1]; // Extract the command from the arguments.
    
    // Match the command string and execute the corresponding function.
    if (strcmp(command, "mkfs") == 0) {
        return cmd_mkfs();
    }
    else if (strcmp(command, "mkdir_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s mkdir_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_mkdir_fs(argv[2]);
    }
    else if (strcmp(command, "create_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s create_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_create_fs(argv[2]);
    }
    else if (strcmp(command, "write_fs") == 0) {
        if (argc != 4) {
            printf("Usage: %s write_fs <path> <data>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_write_fs(argv[2], argv[3]);
    }
    else if (strcmp(command, "read_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s read_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_read_fs(argv[2]);
    }
    else if (strcmp(command, "ls_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s ls_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_ls_fs(argv[2]);
    }
    else if (strcmp(command, "delete_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s delete_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_delete_fs(argv[2]);
    }
    else if (strcmp(command, "rmdir_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s rmdir_fs <path>\n", argv[0]);
            return 1; // Return error code if arguments are missing.
        }
        return cmd_rmdir_fs(argv[2]);
    }
    else {
        printf("Unknown command: %s\n", command); // Handle unknown commands.
        print_usage(argv[0]); // Print usage instructions.
        return 1; // Return error code.
    }
}