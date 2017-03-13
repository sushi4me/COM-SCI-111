#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = python

FILES = lab3b.py
OUT = lab3b_check.txt

TAR_LABEL = lab3b-204401093
TAR_FILES = lab3b.py Makefile README

.SILENT:
default:
	$(CC) $(FILES)
	sort -o $(OUT) $(OUT)

run: default

tarball:
	rm -f $(TAR_LABEL).tar.gz
	tar -czf $(TAR_LABEL).tar.gz $(TAR_FILES)
	rm -f $(TAR_FILES)

clean:
	rm -f $(OUT)
