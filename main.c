#include "fs.h"
#include "disk.h"
#include <stdio.h>

int main() {
       mkfs_fs("disk.img");
       disk_open("disk.img");
       printf("Filesystem created on disk.img\n");

       if (mkdir_fs("/src") == 0) {
           printf("Created /src\n");
           mkdir_fs("/src/subdir");
           printf("Created /src/subdir\n");
       } else {
           printf("Failed to create /src\n");
       }
   
       disk_close();
       return 0;
}
   