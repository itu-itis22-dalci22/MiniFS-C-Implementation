#include "fs.h"
#include "disk.h"
#include <stdio.h>

int main() {
    mkfs_fs("disk.img");
    disk_open("disk.img");

    load_bitmap();

    int b = allocate_block();
    printf("Allocated block: %d\n", b);

    printf("Block %d is now: %s\n", b, is_block_free(b) ? "free" : "used");

    free_block(b);
    printf("Block %d after free: %s\n", b, is_block_free(b) ? "free" : "used");

    disk_close();
    return 0;
}
