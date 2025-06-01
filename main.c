#include "fs.h"
#include "disk.h"
#include <stdio.h>

int main() {
    mkfs_fs("disk.img");
    disk_open("disk.img");

    Inode root;
    read_inode(0, &root);
    printf("Root inode: valid=%d dir=%d size=%d\n",
           root.is_valid, root.is_directory, root.size);

    int inum = allocate_inode();
    printf("Allocated inode: %d\n", inum);

    Inode new_inode;
    read_inode(inum, &new_inode);
    printf("New inode: valid=%d dir=%d size=%d\n",
           new_inode.is_valid, new_inode.is_directory, new_inode.size);

    disk_close();
    return 0;
}
