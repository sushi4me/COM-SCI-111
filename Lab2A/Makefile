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

DIST_LABEL = lab2-204401093
DIST_FILES = Makefile lab2_add.c lab2_list.c \
	SortedList.h SortedList.c \
	lab2_add.csv lab2_list.csv README.txt \
	lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png \
	lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png \
	lab2_add.gp lab2_list.gp

RM_FILES = lab2_add lab2_list \
	mlab2_add.csv mlab2_list.csv \
	mlab2_add-1.png mlab2_add-2.png mlab2_add-3.png mlab2_add-4.png mlab2_add-5.png \
	mlab2_list-1.png mlab2_list-2.png mlab2_list-3.png mlab2_list-4.png \

T_LIST1 := 1 2 4 8 12
T_LIST2 := 2 4 8 12

I_LIST1 := 100 1000 10000 100000
I_LIST2 := 100 1000 10000
I_LIST3 := 10 100 1000 10000 20000
I_LIST4 := 10 100 1000
I_LIST5 := 2 4 8 16 32

CONST1 := 1000
CONST2 := 10000
CONST3 := 12
CONST4 := 32

ADD_CSV = $(ADD_CSV)
LIST_CSV = m$(LIST_CSV)

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

graphs:
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

tests:
	# PART 1
	# add w/o yield
	@for a in $(T_LIST1); do						\
		for b in $(I_LIST1); do		 				\
			./lab2_add --t $${a} --i $${b} >> $(ADD_CSV); 	\
		done 								\
	done
	# add w/ yield
	@for c in $(T_LIST1); do 						\
		for d in $(I_LIST1); do 					\
			./lab2_add --y --t $${c} --i $${d} >> $(ADD_CSV); 	\
		done 								\
	done
	# mutex w/o yield
	@for e in $(T_LIST1); do						\
		for f in $(CONST2); do		 				\
			./lab2_add --t $${e} --i $${f} --s m >> $(ADD_CSV); 	\
		done 								\
	done
	# mutex w/ yield
	@for g in $(T_LIST2); do 							\
		for h in $(CONST2); do 							\
			./lab2_add --y --t $${g} --i $${h} --s m >> $(ADD_CSV); 	\
		done 									\
	done
	# spin w/o yield
	@for i in $(T_LIST1); do						\
		for j in $(CONST2); do		 				\
			./lab2_add --t $${i} --i $${j} --s s >> $(ADD_CSV); 	\
		done 								\
	done
	# spin w/ yield
	@for k in $(T_LIST2); do 							\
		for l in $(CONST1); do 							\
			./lab2_add --y --t $${k} --i $${l} --s s >> $(ADD_CSV); 	\
		done 									\
	done
	# compare w/o yield
	@for m in $(T_LIST1); do						\
		for n in $(CONST2); do		 				\
			./lab2_add --t $${m} --i $${n} --s c >> $(ADD_CSV); 	\
		done 								\
	done
	# compare w/ yield
	@for o in $(T_LIST2); do	 						\
		for p in $(CONST2); do	 						\
			./lab2_add --y --t $${c} --i $${d} --s c >> $(ADD_CSV); 	\
		done 									\
	done
	# PART 2
	# time per operation vs number of iterations
	./lab2_list --threads=1  --iterations=10	      >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=100	      >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=1000	      >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=10000	      >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=20000	      >> $(LIST_CSV)
	# conflicts w/o yielding
	-./lab2_list --threads=2  --iterations=10             >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=100            >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=1000           >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=10             >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=100            >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=1000           >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=10             >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=100            >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=1000           >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=10             >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=100            >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=1000           >> $(LIST_CSV)
	# insert conflicts
	-./lab2_list --threads=2  --iterations=2   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=4   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=8   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=16  --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=32  --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=2   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=4   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=8   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=16  --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=2   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=4   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=8   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=16  --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=2   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=4   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=8   --yield=i  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=16  --yield=i  >> $(LIST_CSV)
	# delete conflicts
	-./lab2_list --threads=2  --iterations=2   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=4   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=8   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=16  --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=32  --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=2   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=4   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=8   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=16  --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=2   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=4   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=8   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=16  --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=2   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=4   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=8   --yield=d  >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=16  --yield=d  >> $(LIST_CSV)
	# insert/lookup conflicts
	-./lab2_list --threads=2  --iterations=2   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=4   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=8   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=16  --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=32  --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=2   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=4   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=8   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=16  --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=2   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=4   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=8   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=16  --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=2   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=4   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=8   --yield=il >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=16  --yield=il >> $(LIST_CSV)
	# delete/lookup conflicts
	-./lab2_list --threads=2  --iterations=2   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=4   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=8   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=16  --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=2  --iterations=32  --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=2   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=4   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=8   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=4  --iterations=16  --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=2   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=4   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=8   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=8  --iterations=16  --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=2   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=4   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=8   --yield=dl >> $(LIST_CSV)
	-./lab2_list --threads=12 --iterations=16  --yield=dl >> $(LIST_CSV)
	# efficacy of mutex and spin-lock synchronization
	./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> $(LIST_CSV)
	# performance
	./lab2_list --threads=1  --iterations=1000          >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=2  --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=4  --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=8  --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=16 --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=24 --iterations=1000 --sync=m >> $(LIST_CSV)
	./lab2_list --threads=1  --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=2  --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=4  --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=8  --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=12 --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=16 --iterations=1000 --sync=s >> $(LIST_CSV)
	./lab2_list --threads=24 --iterations=1000 --sync=s >> $(LIST_CSV)	

clean:
	rm -f $(DIST_LABEL).tar.gz $(RM_FILES)
