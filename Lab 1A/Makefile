CC = gcc
CFLAGS = -g
FILES = main.c
OUT = simpsh
DIST_LABEL = lab1-nathankong
DIST_FILES = main.c Makefile README test.sh

.SILENT:
all: $(FILES)
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)

check:
	@echo "Testing?!"

dist:
	rm -f $(DIST_LABEL).tar.gz simpsh
	tar -czf $(DIST_LABEL).tar.gz $(DIST_FILES)

clean:
	rm -f $(DIST_LABEL).tar.gz simpsh
