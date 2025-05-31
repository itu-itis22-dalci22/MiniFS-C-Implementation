all: fs

fs: main.o disk.o
	gcc -o fs main.o disk.o

%.o: %.c
	gcc -c $<

clean:
	rm -f *.o fs
