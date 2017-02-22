#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g -std=c99 -Wall

FILES = lab2_add.c
OUT = lab2_add

DIST_LABEL = lab1-nathankong
DIST_FILES = Makefile lab2_add.c lab2_list.c \
	SortedList.h SortedList.c \
	lab2_add.csv lab2_list.csv README.txt

TEST_FILES = 
TEST_FOLDERS = 

.SILENT:
all:
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)

dist:
	rm -f $(DIST_LABEL).tar.gz
	mkdir $(DIST_LABEL)
	cp $(DIST_FILES) ./$(DIST_LABEL)	
	tar -czf $(DIST_LABEL).tar.gz ./$(DIST_LABEL)
	rm -rf $(DIST_LABEL)

clean:
	rm -f $(DIST_LABEL).tar.gz lab2_add
