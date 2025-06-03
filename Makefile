# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Target binary
all: mini_fs

# Main executable
mini_fs: main.o disk.o fs.o
	$(CC) $(CFLAGS) -o mini_fs main.o disk.o fs.o

# Compile source files
main.o: main.c fs.h disk.h
	$(CC) $(CFLAGS) -c main.c

disk.o: disk.c disk.h
	$(CC) $(CFLAGS) -c disk.c

fs.o: fs.c fs.h disk.h
	$(CC) $(CFLAGS) -c fs.c

# Run automated tests
check: mini_fs
	@echo "[Running automated test...]"
	@rm -f tests/output.txt
	@while IFS= read -r line; do \
		$$line >> tests/output.txt; \
	done < tests/commands.txt
	@diff -u tests/expected_output.txt tests/output.txt || { echo "Output mismatch"; exit 1; }
	@echo "Output matches expected."


# Clean build artifacts
clean:
	rm -f *.o mini_fs disk.img tests/output.txt
