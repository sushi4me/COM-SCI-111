#------------------------------------------------------
#	Nathan Kong
#	204 401 093
#------------------------------------------------------

CC = gcc
CFLAGS = -g -pthread -std=c99 -Wall

FILES = lab2_add.c
FILES2 = lab2_list.c SortedList.c
OUT = lab2_add
OUT2 = lab2_list

DIST_LABEL = lab2b-204401093
DIST_FILES = Makefile lab2_add.c lab2_list.c \
	SortedList.h SortedList.c \
	lab_2b_list.csv README.txt \
	lab2b.gp \
	lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png \
	profile.gperf

RM_FILES = lab2_add lab2_list \
	mlab_2b_list.csv lab_2b_4.csv lab_2b_5.csv\
	mlab2b_1.png mlab2b_2.png mlab2b_3.png mlab2b_4.png mlab2b_5.png \
	mprofile.gperf

T_LIST1 := 1 2 4 8 12
T_LIST2 := 1 2 4 8 12 16 24
T_LIST3 := 1 2 4 8 12 16

I_LIST1 := 100 1000 10000 100000
I_LIST2 := 100 1000 10000
I_LIST3 := 10 100 1000 10000 20000
I_LIST4 := 10 100 1000
I_LIST5 := 2 4 8 16 32
I_LIST6 := 1 2 4 8 16
I_LIST7 := 10 20 40 80

CONST1 := 1000
CONST2 := 10000
CONST3 := 12
CONST4 := 32

LAB2B_CSV = mlab_2b_list.csv
4_CSV = lab_2b_4.csv
5_CSV = lab_2b_5.csv

.SILENT:
default:
	$(CC) $(CFLAGS) -o $(OUT) $(FILES)
	$(CC) $(CFLAGS) -o $(OUT2) $(FILES2)

build: default

tarball:
	rm -f $(DIST_LABEL).tar.gz
	mkdir $(DIST_LABEL)
	cp $(DIST_FILES) ./$(DIST_LABEL)	
	tar -czf $(DIST_LABEL).tar.gz ./$(DIST_LABEL)
	rm -rf $(DIST_LABEL)

profile:
	-rm -f ./raw.gperf
	LD_PRELOAD=/usr/local/lib/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --t 12 --i 1000 --s s
	pprof --text ./lab2_list ./raw.gperf > mprofile.gperf
	pprof --list=runTestThread ./lab2_list ./raw.gperf >> mprofile.gperf
	rm -f ./raw.gperf

graphs:
	gnuplot lab2b.gp

clean:
	rm -f $(DIST_LABEL).tar.gz $(RM_FILES)

tests:
	# PLOT 1 & 2
	# mutex-synchronized adds
	@for a in $(T_LIST1); do						\
		for b in $(CONST2); do		 				\
			./lab2_add --t $${a} --i $${b} --s m>> $(LAB2B_CSV); 	\
		done 								\
	done
	# spin-lock synchronized adds
	@for c in $(T_LIST1); do 						\
		for d in $(CONST2); do 						\
			./lab2_add --t $${c} --i $${d} --s s>> $(LAB2B_CSV);	\
		done 								\
	done
	# mutex synchronized list operations
	@for e in $(T_LIST2); do						\
		for f in $(CONST1); do		 				\
			./lab2_list --t $${e} --i $${f} --s m >> $(LAB2B_CSV); 	\
		done 								\
	done
	# spin-lock synchronized list operations
	@for g in $(T_LIST2); do 						\
		for h in $(CONST1); do 						\
			./lab2_list --t $${g} --i $${h} --s s >> $(LAB2B_CSV); 	\
		done 								\
	done
	# PLOT 3
	# no sync list operations
	-./lab2_list --t 1	--i 1 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 2 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 4 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 8 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 16 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 1 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 2 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 4 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 8 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 16 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 1 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 2 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 4 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 8 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 16 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 1 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 2 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 4 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 8 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 16 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 1 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 2 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 4 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 8 	--y di 	--l 4 		>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 16 	--y di 	--l 4 		>> $(LAB2B_CSV)
	# mutex synchronized list operations
	-./lab2_list --t 1	--i 10 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 20 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 40 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 80 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 10 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 20 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 40 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 80 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 10 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 20 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 40 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 80 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 10 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 20 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 40 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 80 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 10 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 20 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 40 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 80 	--y di 	--l 4 	--s m	>> $(LAB2B_CSV)
	# spin-lock synchronized list operations
	-./lab2_list --t 1	--i 10 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 20 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 40 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 1	--i 80 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 10 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 20 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 40 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 4	--i 80 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 10 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 20 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 40 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 8	--i 80 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 10 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 20 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 40 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 12	--i 80 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 10 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 20 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 40 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	-./lab2_list --t 16	--i 80 	--y di 	--l 4 	--s s	>> $(LAB2B_CSV)
	# PLOT 4
	# mutex with varying lists
	-./lab2_list --t 1	--i 1000	--l 1 	--s m	>> $(4_CSV)
	-./lab2_list --t 1	--i 1000	--l 4 	--s m	>> $(4_CSV)
	-./lab2_list --t 1	--i 1000	--l 8 	--s m	>> $(4_CSV)
	-./lab2_list --t 1	--i 1000	--l 16 	--s m	>> $(4_CSV)
	-./lab2_list --t 2	--i 1000	--l 1 	--s m	>> $(4_CSV)
	-./lab2_list --t 2	--i 1000	--l 4 	--s m	>> $(4_CSV)
	-./lab2_list --t 2	--i 1000	--l 8 	--s m	>> $(4_CSV)
	-./lab2_list --t 2	--i 1000	--l 16 	--s m	>> $(4_CSV)
	-./lab2_list --t 4	--i 1000	--l 1 	--s m	>> $(4_CSV)
	-./lab2_list --t 4	--i 1000	--l 4 	--s m	>> $(4_CSV)
	-./lab2_list --t 4	--i 1000	--l 8 	--s m	>> $(4_CSV)
	-./lab2_list --t 4	--i 1000	--l 16 	--s m	>> $(4_CSV)
	-./lab2_list --t 8	--i 1000	--l 1 	--s m	>> $(4_CSV)
	-./lab2_list --t 8	--i 1000	--l 4 	--s m	>> $(4_CSV)
	-./lab2_list --t 8	--i 1000	--l 8 	--s m	>> $(4_CSV)
	-./lab2_list --t 8	--i 1000	--l 16 	--s m	>> $(4_CSV)
	-./lab2_list --t 16	--i 1000	--l 1 	--s m	>> $(4_CSV)
	-./lab2_list --t 16	--i 1000	--l 4 	--s m	>> $(4_CSV)
	-./lab2_list --t 16	--i 1000	--l 8 	--s m	>> $(4_CSV)
	-./lab2_list --t 16	--i 1000	--l 16 	--s m	>> $(4_CSV)
	cat $(4_CSV) >> $(LAB2B_CSV)
	# PLOT 5
	# spin-lock with varying lists
	-./lab2_list --t 1	--i 1000	--l 1 	--s s	>> $(5_CSV)
	-./lab2_list --t 1	--i 1000	--l 4 	--s s	>> $(5_CSV)
	-./lab2_list --t 1	--i 1000	--l 8 	--s s	>> $(5_CSV)
	-./lab2_list --t 1	--i 1000	--l 16 	--s s	>> $(5_CSV)
	-./lab2_list --t 2	--i 1000	--l 1 	--s s	>> $(5_CSV)
	-./lab2_list --t 2	--i 1000	--l 4 	--s s	>> $(5_CSV)
	-./lab2_list --t 2	--i 1000	--l 8 	--s s	>> $(5_CSV)
	-./lab2_list --t 2	--i 1000	--l 16 	--s s	>> $(5_CSV)
	-./lab2_list --t 4	--i 1000	--l 1 	--s s	>> $(5_CSV)
	-./lab2_list --t 4	--i 1000	--l 4 	--s s	>> $(5_CSV)
	-./lab2_list --t 4	--i 1000	--l 8 	--s s	>> $(5_CSV)
	-./lab2_list --t 4	--i 1000	--l 16 	--s s	>> $(5_CSV)
	-./lab2_list --t 8	--i 1000	--l 1 	--s s	>> $(5_CSV)
	-./lab2_list --t 8	--i 1000	--l 4 	--s s	>> $(5_CSV)
	-./lab2_list --t 8	--i 1000	--l 8 	--s s	>> $(5_CSV)
	-./lab2_list --t 8	--i 1000	--l 16 	--s s	>> $(5_CSV)
	-./lab2_list --t 16	--i 1000	--l 1 	--s s	>> $(5_CSV)
	-./lab2_list --t 16	--i 1000	--l 4 	--s s	>> $(5_CSV)
	-./lab2_list --t 16	--i 1000	--l 8 	--s s	>> $(5_CSV)
	-./lab2_list --t 16	--i 1000	--l 16 	--s s	>> $(5_CSV)
	cat $(5_CSV) >> $(LAB2B_CSV)
