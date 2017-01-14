CC=gcc

.SILENT:

default: main.c
	@echo "Compiling!"
	$(CC) -o main -g main.c
check: main
	@echo "Checking..."
clean:
	@echo "Cleaning up..."
	rm -f main *.o
dist:
	@echo "Packing everything up!"
	rm -f lab0-204401093.tar.gz
	tar -xzcf lab0-204401093.tar.gz *.c Makefile *.jpeg README
