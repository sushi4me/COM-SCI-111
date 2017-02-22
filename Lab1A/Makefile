#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g

FILES = main.c
OUT = simpsh
DIST_LABEL = lab1-nathankong
DIST_FILES = main.c Makefile README
TEST_FILES = READIN.txt testout.txt testerr.txt

.SILENT:
all: $(FILES)
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)

check:
	echo "Hello world!" > READIN.txt
	touch testout.txt
	touch testerr.txt
	printf "TEST 1: rdonly, wronly, command --- \t\t"
	./simpsh --rdonly READIN.txt --wronly testout.txt \
	--wronly testerr.txt --command 0 1 2 cat > /dev/null 2>&1
	diff testout.txt READIN.txt && printf "PASS\n"
	printf "TEST 2: No existing file for wronly --- \t"
	./simpsh --wronly no_test.txt > /dev/null 2>&1 && \
	[ $$? -ne 0 ] || printf "PASS\n"
	printf "TEST 3: Error in command --- \t\t\t"
	> testout.txt
	> testerr.txt
	./simpsh --rdonly READIN.txt --wronly testout.txt \
	--wronly testerr.txt --no_such_thing \
	--command 0 1 2 cat > /dev/null 2>&1
	diff testout.txt READIN.txt && printf "PASS\n"

dist:
	rm -f $(DIST_LABEL).tar.gz simpsh
	mkdir $(DIST_LABEL)
	cp $(DIST_FILES) ./$(DIST_LABEL)	
	tar -czf $(DIST_LABEL).tar.gz ./lab1-nathankong

clean:
	rm -f $(DIST_LABEL).tar.gz simpsh $(TEST_FILES)
