Name:	Nathan Kong
UID:	204 401 093
Title:	Lab 2A

Notes:
	* 'make clean' will only removed *.png and *.csv made from the Makefile
	* SortedList_lookup assumes the user passes the HEAD of the list
	* SortedList_lookup can search for the HEAD of the list (key == NULL)
	* make graphs may sometimes not work due to missing lines in *.csv

Files:
	* lab2_add.c:
		Compiles into the program lab2_add.
		
		There are four options: 
			thread number, iteration number, yields, and protection
			for threads (mutex, spin-lock, and compare and swap).

	* lab2_list.c:
		Compiles into the program lab2_list.
		
		There are four options: 
			thread number, iteration number, yields, delete, or 
			lookup, and protection for threads (mutex and 
			spin-lock).

	* SortedList.h:
		This is the provided header file for a doubly-linked list.

	* SortedList.c:
		Implements the doubly-linked list.

	* Makefile:
		make: 
			Compiles and builds lab2_add and lab2_list.

		make build:
			Same as make default.

		make tests:
			Runs lab2_add and lab2_list with tests, creates *.csv.

		make graphs:
			Runs gnuplot to plot *.csv.

		make tarball:
			Packs and makes tar.gz.

		make clean:
			Removes all files created by make.

	* lab2_add.csv:
		Generated data from lab2_add runs.

	* lab2_list.csv:
		Generated data from lab2_list runs.

	* lab2_add.gp:
		Generates plots from lab2_add.csv, edited to have different
		file names than those in make tarball.	

	* lab2_list.gp:
		Generates plots from lab2_list.csv, edited to have different
		file names than those in make tarball.	

	* lab2_add-1.png:
		Threads and iterations that run without failure.

	* lab2_add-2.png:
		Average time per operation with and without yields.

	* lab2_add-3.png:
		Average time per (single threaded) operation vs. the 
		number of iterations.

	* lab2_add-4.png:
		Threads and iterations that can run successfully with 
		yields under each of the three synchronization methods.

	* lab2_add-5.png:
		Average time per (multi-threaded) operation vs. the 
		number of threads, for all four versions of the add 
		function.

	* lab2_list-1.png:
		Average time per (single threaded) unprotected 
		operation vs. number of iterations (illustrating the 
		correction of the per-operation cost for the list 
		length).

	* lab2_list-2.png:
		Threads and iterations required to generate a failure
		(with and without yields).

	* lab2_list-3.png:
		Protected iterations that run without failure.

	* lab2_list-4.png:
		Average time per operation (for unprotected, mutex, and
		spin-lock) vs. number of threads.

Question 2.1.1 - causing conflicts:
	We only start to see errors in the program when there are enough
	threads running in parallel with one another to compete for 'result'.
	With a small number of iterations it could be the case that the threads
	finish before the next thread starts therefore they do not collide.  By
	running these threads long enough (with a large iteration) we start to
	see more incorrect results.

Question 2.1.2 - cost of yielding:
	Whenever a thread yields, there is a context switch which is very
	expensive in terms of time.  We cannot get an accurate per-operation
	timing for the 'add' function with --yield because most of the time is
	being spent on context switches.

Question 2.1.3 - measurement errors:
	The average cost per operation decreases as the number of iterations
	increases because each thread created is getting more done.  To create
	a thread is expensive therefore having it run for a short period of
	time would not be ideal.  Imagine a situation in which you want to
	have 10000 threads run once, the cost of creating these threads would
	outweigh the efficieny you get from splitting a process.

	The "correct" cost would appear when we see that the cost per iteration
	saturates (should be asymptotic).  The cost will decrease as you spawn
	more threads to cut down run time however there will be a point in
	which the time to create a thread outweighs the time gained in
	efficiency.  The more iterations we have, the more threads we can spawn
	and keep the cost within reasonable performance.  We see this
	asymptotic effect at around 10000 iterations in 'lab2_add-2.png'.

Question 2.1.4 - costs of serialization:
	For a low number of threads, we do not experience as many collisions
	between parallel threads.  It may also be the case that these threads
	are able to finish before too many threads are requesting for CPU time
	by not yielding.

	As we increase the number of threads, protected operations will slow	
	down as more threads are colliding with one another via checks and 
	stalls (spinning).  

	When a spin lock fails to obtain a lock it puts itself into a constant
	loop which wastes CPU time for others to do actual work.  By having
	many of them running such as in this case, a majority will be 
	spinning rather than actually conducting any useful operation.

Question 2.2.1 - scalability of Mutex
	The cost of synchronization for lists are much more expensive than that
	of a simple add.  The linked list operations were locked for longer
	periods of time which caused a lot of collisions between threads.  We
	pay for the price of synchronization in linked lists with more run
	time.

Question 2.2.2 - scalability of spin locks
	With a low number of threads, spin locks should yield a better
	performance than mutex locks because the time it takes to access mutex
	locks seem to outweigh the spin time of a spin lock.  However as we
	increase the number of threads we see a performance drop in spin locks
	as many of them are using up CPU cycles to monitor something 
	constantly.  This is more apparent in PART 1 than PART 2.
	
