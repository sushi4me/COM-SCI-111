#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = 		gcc
CFLAGS1 = 	-g -Wall -lm -lmraa -o lab4_1
CFLAGS2 =	-g -Wall -lm -lmraa -pthread -o lab4_2
CFLAGS3 =	-g -Wall -lm -lmraa -pthread -o lab4_3 -lssl -lcrypto

TAR_LABEL = 	lab4-204401093
TAR_FILES = 	part1_main.c \
		part2_main.c \
		part3_main.c \
		lab4_1.log \
		lab4_2.log \
		lab4_3.log \
		Makefile \
		README

OUT = 		lab4_1 \
		lab4_2 \
		lab4_3

LOGS =		lab4_1.log \
		lab4_2.log \
		lab4_3.log

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
		./lab4_1;			\
	else					\
		$(CC) $(CFLAGS1) part1_main.c; 	\
		./lab4_1;			\
	fi
run2:
	@if [ -e part2_main ]; then		\
		./lab4_2;			\
	else					\
		$(CC) $(CFLAGS2) part2_main.c; 	\
		./lab4_2;			\
	fi

run3:
	@if [ -e part3_main ]; then		\
		./lab4_3;			\
	else					\
		$(CC) $(CFLAGS3) part3_main.c; 	\
		./lab4_3;			\
	fi

tarball:
	rm -f $(TAR_LABEL).tar.gz
	tar -czf $(TAR_LABEL).tar.gz $(TAR_FILES)

clean:
	rm -f $(OUT) $(LOGS)
