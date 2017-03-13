#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = 		gcc
CFLAGS1 = 	-g -Wall -lm -lmraa -o part1_main
CFLAGS2 =	-g -Wall -lm -lmraa -pthread -o part2_main
CFLAGS3 =	-g -Wall -lm -lmraa -pthread -o part3_main -lssl -lcrypto

TAR_LABEL = 	lab4-204401093
TAR_FILES = 	part1_main.c \
		part2_main.c \
		part3_main.c \
		log2.txt \
		log3.txt \
		Makefile \
		README

OUT = 		part1_main \
		part2_main \
		part3_main

LOGS =		part1_log.txt \
		part2_log.txt \
		part3_log.txt

.SILENT:

default: part1 part2 part3

all: default

part1:
	$(CC) $(CFLAGS1) part1_main.c

part2:
	$(CC) $(CFLAGS2) part2_main.c

part3:
	$(CC) $(CFLAGS3) part3_main.c

run1:
	@if [ -e part1_main ]; then		\
		./part1_main;			\
	else					\
		$(CC) $(CFLAGS1) part1_main.c; 	\
		./part1_main;			\
	fi
run2:
	@if [ -e part2_main ]; then		\
		./part2_main;			\
	else					\
		$(CC) $(CFLAGS2) part2_main.c; 	\
		./part2_main;			\
	fi

run3:
	@if [ -e part3_main ]; then		\
		./part3_main;			\
	else					\
		$(CC) $(CFLAGS3) part3_main.c; 	\
		./part3_main;			\
	fi

tarball:
	rm -f $(TAR_LABEL).tar.gz
	tar -czf $(TAR_LABEL).tar.gz $(TAR_FILES)

clean:
	rm -f $(OUT) $(LOGS)
