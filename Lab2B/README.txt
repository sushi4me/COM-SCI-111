Name:	Nathan Kong
UID:	204 401 093
Title:	Lab 2B

Notes:
	* Files created by #make# will start with letter 'm' to differentiate 
	  from originals.
	* Sometimes running #make tests# will occur a SIGSEGV, I may have fixed
	  this as it has not come up since that one time?
	* I had install gperftools onto my own laptop, I was having a very 
	  difficult time installing it onto the linux severs as libunwind would
	  not install properly either!

Files:
	* lab2_add.c:
		Compiles into the program lab2_add.

	* lab2_list.c:
		Compiles into the program lab2_list.
		
		Now uses partitioned lists to improve performance!
		
		Each list is given a lock therefore we do not lock a single
		list when performing operations to an element (located in
		that list).

	* SortedList.h:
		This is the provided header file for a doubly-linked list.

	* SortedList.c:
		Implements the doubly-linked list.

	* Makefile:
		make: 
			Compiles and builds lab2_add and lab2_list.

		make build:
			Same as make default.

		make profile:
			Attempts to run gperftools that was installed on
			**MY PERSONAL LAPTOP** therefore the commands deviate
			from those seen on Piazza.  However it works!

		make tests:
			Runs tests and stows them into a CSV file for graphs.

		make graphs:
			Runs gnuplot to plot CSV files, yields 5 plots.

		make tarball:
			Packs and makes tar.gz.

		make clean:
			Removes all files created by make.

Question 2.3.1 - cycles in the basic implementation:
	In 1 and 2-thread tests for add and locking, these two spend roughly
	equal times waiting for locks however the outlier are the list
	operations.  These list operations are much more complicated than
	accessing a single variable for incrementing.  In list operations we
	are working with structures.  I would not say we spent a lot of time
	waiting for locks because between (at most) 2 threads there would not
	be a lot of collisions for them during operation.

	In the high-thread tests we see that more time is being spent on
	waiting to claim mutex locks because there are many times more threads
	then there are variables/lists that are avaialble for atomic access.

Question 2.3.2 - execution profiling:
	In the tests with gperftools we find that the most time is being spent
	on waiting for spin-locks to finally become avaialable for lists.

	This is the case because instead of doing something else spin-locks
	tell all other threads spinning to repeatedly check the resource.  This
	wastes CPU cycles, thus hindering actual processing time.

Question 2.3.3 - mutex wait time:
	With more threads it is more likely for another thread to have taken
	the lock beforehand, and there will be longer lines for locks.

	Completion time is hindered by the factors of blocking, unblocking, and
	context switching between contending threads.  This does not change as
	drastically as lock-wait time because work is always being done by some
	thread.

	Waiting time can go up faster/higher than completion time because wait
	time is measured by each single thread, whereas completion time is
	measured in the wholistic sense.  In the waiting scenario as many as
	N-1 threads can be waiting for a lock if they are all trying to perform
	the same operation.

Question 2.3.4 - performance of partitioned lists:
	If we fix the number of threads and increase the number of lists, then
	we should see an increase in performance as threads are less likely 
	waiting for the same lock (almost as if each thread has its own list
	to work on).

	Once the number of lists exceeds the number of threads in operation,
	the gain in performance starts to drop off because we are not avoiding
	any further collisions.  We are simply furthering the likelihood of
	thread collision to be zero, but these gains in performance are small.

	Some factors that determine the likelihood of collision:
	* The number of operating threads at a given time.
	* The amount of time spent in the critical section before releasing the
	  lock.
	* The nature of the tasks each thread is given.
	By partitioning lists we reduce the overall likelihood of threads
	competing for locks but also reduce the length of the lists themselves.
	In doing so we lessen the load of operations such as finding the length
	and look-ups.