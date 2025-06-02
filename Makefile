# Target binary
all: fs

# Main executable built from object files
fs: main.o disk.o fs.o
	gcc -o fs main.o disk.o fs.o

# Compile .c to .o with header dependencies
main.o: main.c fs.h disk.h
	gcc -c main.c

disk.o: disk.c disk.h
	gcc -c disk.c

fs.o: fs.c fs.h disk.h
	gcc -c fs.c

# Clean up build artifacts
clean:
	rm -f *.o fs disk.img