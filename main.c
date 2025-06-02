#include "fs.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>

int main() {
    const char *disk_name = "disk.img";
    const char *dir_path = "/docs";
    const char *file_path = "/docs/report.txt";
    const char *test_content = "This is a test file written to MiniFS!";
    char read_buffer[1024] = {0};  // enough to read back the file

    // Step 1: Format the disk
    if (mkfs_fs(disk_name) != 0) {
        printf("Failed to format disk.\n");
        return 1;
    }

    // Step 2: Open the disk
    if (disk_open(disk_name) != 0) {
        printf("Failed to open disk.\n");
        return 1;
    }

    // Step 3: Create a directory "/docs"
    if (mkdir_fs(dir_path) == 0) {
        printf("Directory %s created successfully.\n", dir_path);
    } else {
        printf("Failed to create directory %s.\n", dir_path);
        return 1;
    }

    // Step 4: Create a file "/docs/report.txt"
    if (create_fs(file_path) == 0) {
        printf("File %s created successfully.\n", file_path);
    } else {
        printf("Failed to create file %s.\n", file_path);
        return 1;
    }

    // Step 5: Write content to the file
    if (write_fs(file_path, test_content, strlen(test_content)) >= 0) {
        printf("Wrote content to %s.\n", file_path);
    } else {
        printf("Failed to write to file %s.\n", file_path);
        return 1;
    }

    // Step 6: Read content back from the file
    int bytes_read = read_fs(file_path, read_buffer, sizeof(read_buffer));
    if (bytes_read >= 0) {
        read_buffer[bytes_read] = '\0';  // Null-terminate just in case
        printf("Read back %d bytes: \"%s\"\n", bytes_read, read_buffer);
    } else {
        printf("Failed to read from file %s.\n", file_path);
        return 1;
    }

    // ls // List the contents of the directory
    DirectoryEntry entries[10];  // Assuming we won't have more than 10 entries
    int num_entries = ls_fs("/", entries, 10);
    if (num_entries >= 0) {
        printf("Contents of %s:\n", "/");
        for (int i = 0; i < num_entries; i++) {
            printf(" - %s (inode: %u)\n", entries[i].name, entries[i].inum);
        }
    } else {
        printf("Failed to list contents of directory %s.\n", "/");
    }
    
    // Step 7: Delete the file
    if (delete_fs("/docs/report.txt") == 0) {
        printf("Deleted file successfully.\n");
    } else {
        printf("Failed to delete file.\n");
    }

    // Step 8: Remove the directory
    if (rmdir_fs(dir_path) == 0) {
            printf("Removed directory %s successfully.\n", dir_path);
    } else {
            printf("Failed to remove directory %s.\n", dir_path);
    }

    // Step 8: Close the disk
    disk_close();
    return 0;
}
   