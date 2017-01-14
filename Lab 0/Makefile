CC = gcc
files = main.c Makefile *.png README
.SILENT:

default: main.c
	$(CC) -g main.c -o lab0
check: lab0 catch_segfault_test IO_test
	# Check tests
clean:
	rm -f lab0 *.o log.txt lab0-204401093.tar.gz
dist:
	rm -f lab0-204401093.tar.gz
	tar -czf lab0-204401093.tar.gz $(files)
# Tests
catch_segfault_test:
	# &> is for stdout and stderr redirection
	# The RHS only executes if LHS is non-zero, 0 is TRUE	
	./lab0 --catch --segfault > /dev/null || [ $$? -eq 3 ] || \
	echo "FAILED: catch & segfault test" >> log.txt
IO_test:
	echo "Hello Nathan Kong!\nThis is for CS 111." > input.txt
	./lab0 --input=input.txt --output=output.txt
	diff input.txt output.txt > /dev/null || \
	echo "FAILED: IO test [$$?]" >> log.txt
	rm -f input.txt
	rm -f output.txt
