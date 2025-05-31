# Target binary
all: fs

# Main executable built from all object files
fs: main.o disk.o fs.o
	gcc -o fs main.o disk.o fs.o

# Compile any .c file into .o
%.o: %.c
	gcc -c $<

# Clean up build artifacts
clean:
	rm -f *.o fs disk.img
