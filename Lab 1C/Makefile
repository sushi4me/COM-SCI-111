#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g -std=c99

FILES = main.c
OUT = simpsh
DIST_LABEL = lab1-nathankong
DIST_FILES = main.c Makefile README lab1C_benchmark.pdf
TEST_FILES = READIN.txt testout.txt testerr.txt empty.txt
TEST_FOLDERS = testing_folder

.SILENT:
all: $(FILES)
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)

dist:
	rm -f $(DIST_LABEL).tar.gz simpsh
	mkdir $(DIST_LABEL)
	cp $(DIST_FILES) ./$(DIST_LABEL)	
	tar -czf $(DIST_LABEL).tar.gz ./$(DIST_LABEL)
	rm -rf $(DIST_LABEL)

clean:
	rm -f $(DIST_LABEL).tar.gz simpsh
