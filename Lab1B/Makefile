#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g

FILES = main.c
OUT = simpsh
DIST_LABEL = lab1-nathankong
DIST_FILES = main.c Makefile test.sh README
TEST_FILES = READIN.txt testout.txt testerr.txt empty.txt
TEST_FOLDERS = testing_folder

.SILENT:
all: $(FILES)
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)

check:
	./test.sh

dist:
	rm -f $(DIST_LABEL).tar.gz simpsh
	mkdir $(DIST_LABEL)
	cp $(DIST_FILES) ./$(DIST_LABEL)	
	tar -czf $(DIST_LABEL).tar.gz ./$(DIST_LABEL)
	rm -rf $(DIST_LABEL)

clean:
	rm -rf $(TEST_FOLDERS)
	rm -f $(DIST_LABEL).tar.gz simpsh $(TEST_FILES)
