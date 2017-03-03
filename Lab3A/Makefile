#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g -std=c99 -Wall -lm

FILES = lab3a.c
OUT = lab3a

TAR_LABEL = lab3a-204401093
TAR_FILES = lab3a.c Makefile README

.SILENT:
default:
	$(CC) $(FILES) $(CFLAGS) -o $(OUT)

build: default

tarball:
	rm -f $(TAR_LABEL).tar.gz
	tar -czf $(TAR_LABEL).tar.gz $(TAR_FILES)
	rm -f $(TAR_FILES)

clean:
	rm -f $(OUT) *.csv
