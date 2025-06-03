#include "fs.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int cmd_mkfs() {
    const char *disk_name = "disk.img";
    
    if (mkfs_fs(disk_name) != 0) {
        printf("Failed to format disk.\n");
        return 1;
    }
    printf("Disk formatted successfully.\n");
    return 0;
}

int cmd_mkdir_fs(const char *path) {
    const char *disk_name = "disk.img";
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    if (mkdir_fs(path) == 0) {
        printf("Directory %s created successfully.\n", path);
    } else {
        printf("Failed to create directory %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_create_fs(const char *path) {
    const char *disk_name = "disk.img";
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    if (create_fs(path) == 0) {
        printf("File %s created successfully.\n", path);
    } else {
        printf("Failed to create file %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_write_fs(const char *path, const char *data) {
    const char *disk_name = "disk.img";
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    if (write_fs(path, data, strlen(data)) >= 0) {
        printf("Wrote content to %s.\n", path);
    } else {
        printf("Failed to write to file %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_read_fs(const char *path) {
    const char *disk_name = "disk.img";
    char read_buffer[1024] = {0};
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    int bytes_read = read_fs(path, read_buffer, sizeof(read_buffer));
    if (bytes_read >= 0) {
        read_buffer[bytes_read] = '\0';
        printf("Read %d bytes from %s: \"%s\"\n", bytes_read, path, read_buffer);
    } else {
        printf("Failed to read from file %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_ls_fs(const char *path) {
    const char *disk_name = "disk.img";
    DirectoryEntry entries[10];
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    int num_entries = ls_fs(path, entries, 10);
    if (num_entries >= 0) {
        printf("Contents of %s:\n", path);
        for (int i = 0; i < num_entries; i++) {
            printf(" - %s (inode: %u)\n", entries[i].name, entries[i].inum);
        }
    } else {
        printf("Failed to list contents of directory %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_delete_fs(const char *path) {
    const char *disk_name = "disk.img";
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    if (delete_fs(path) == 0) {
        printf("Deleted file %s successfully.\n", path);
    } else {
        printf("Failed to delete file %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int cmd_rmdir_fs(const char *path) {
    const char *disk_name = "disk.img";
    
    if (init_fs(disk_name) != 0) {
        printf("Failed to initialize filesystem. Run 'mkfs' first.\n");
        return 1;
    }
    
    int result = 0;
    if (rmdir_fs(path) == 0) {
        printf("Removed directory %s successfully.\n", path);
    } else {
        printf("Failed to remove directory %s.\n", path);
        result = 1;
    }
    
    cleanup_fs();
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "mkfs") == 0) {
        return cmd_mkfs();
    }
    else if (strcmp(command, "mkdir_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s mkdir_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_mkdir_fs(argv[2]);
    }
    else if (strcmp(command, "create_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s create_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_create_fs(argv[2]);
    }
    else if (strcmp(command, "write_fs") == 0) {
        if (argc != 4) {
            printf("Usage: %s write_fs <path> <data>\n", argv[0]);
            return 1;
        }
        return cmd_write_fs(argv[2], argv[3]);
    }
    else if (strcmp(command, "read_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s read_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_read_fs(argv[2]);
    }
    else if (strcmp(command, "ls_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s ls_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_ls_fs(argv[2]);
    }
    else if (strcmp(command, "delete_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s delete_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_delete_fs(argv[2]);
    }
    else if (strcmp(command, "rmdir_fs") == 0) {
        if (argc != 3) {
            printf("Usage: %s rmdir_fs <path>\n", argv[0]);
            return 1;
        }
        return cmd_rmdir_fs(argv[2]);
    }
    else {
        printf("Unknown command: %s\n", command);
        print_usage(argv[0]);
        return 1;
    }
}