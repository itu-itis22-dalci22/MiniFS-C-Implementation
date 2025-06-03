# MiniFS – User-Space File System

This is a user-space file system project for the BLG312E Operating Systems course. The file system operates on a 1MB disk image (`disk.img`) using standard C file I/O.

---

## 🔧 How to Build

To compile the project:

```bash
make
````

To clean up compiled files:

```bash
make clean
```

---

## 🚀 How to Run

To see available commands:

```bash
./mini_fs
```

You will see commands like:

* `mkfs` – Format the disk
* `mkdir_fs <path>` – Create directory
* `rmdir_fs <path>` – Remove directory
* `create_fs <path>` – Create file
* `write_fs <path> "<data>"` – Write to file
* `read_fs <path>` – Read from file
* `delete_fs <path>` – Delete file
* `ls_fs <path>` – List contents of a directory

---

## ✅ How to Test

To run the automated tests:

```bash
make check
```

This will:

* Run commands in `tests/commands.txt`
* Compare the output with `tests/expected_output.txt`

---

## 🗂️ Files

* `fs.c` – Filesystem implementation
* `main.c` – Main function for running commands
* `fs.h` – Function declarations
* `disk.img` – Simulated 1MB disk
* `run_log.txt` – Debug logs for inode/block reuse
* `tests/` – Test inputs and expected outputs

---

## 📌 Note

All operations work on absolute paths (e.g., `/docs/test.txt`). The filesystem supports basic file and directory management using direct block addressing.
