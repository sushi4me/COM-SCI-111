#! /usr/bin/gnuplot

# PLOT PARAMETERS
set terminal png
set datafile separator ","

# PLOT 1
set title "1. Synchronized Throughput"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/s)"
set logscale y 10
set output 'mlab2b_1.png'
set key left top

plot \
     	"< grep add-m lab_2b_list.csv" using ($2):(($4)*1000000000/($5)) \
	title 'adds w/ mutex' with linespoints lc rgb 'red', \
	"< grep add-s lab_2b_list.csv" using ($2):(($4)*1000000000/($5)) \
	title 'adds w/ spin' with linespoints lc rgb 'orange', \
	"< grep list-none-m,[0-9]*,1000,1, lab_2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex list' with linespoints lc rgb 'blue', \
     	"< grep list-none-s,[0-9]*,1000,1, lab_2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin list' with linespoints lc rgb 'green'

# PLOT 2
set title "2. Per Operation Times for List Operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "average time per operation (ns)"
set logscale y 10
set output 'mlab2b_2.png'
set key left top

plot \
     	"< grep list-none-m,[1,2,4,8][4,6]*,1000,1, lab_2b_list.csv" using ($2):($7) \
	title 'average time per op' with linespoints lc rgb 'red',\
     	"< grep list-none-m,[1,2,4,8][4,6]*,1000,1, lab_2b_list.csv" using ($2):($8) \
	title 'wait for lock' with linespoints lc rgb 'orange'

# PLOT 3
set title "3. Number of Successful Iterations for each Synchronization Method"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Iterations"
set logscale y 10
set output 'mlab2b_3.png'
set key left top

plot \
	"<grep list-id-none lab_2b_list.csv" using ($2):($3)\
	title "yield=id" with points lc rgb 'red',\
	"<grep list-id-m lab_2b_list.csv" using ($2):($3)\
	title "mutex" with points lc rgb 'orange',\
	"<grep list-id-s lab_2b_list.csv" using ($2):($3)\
	title "spin-lock" with points lc rgb 'blue'

# PLOT 4
set title "4. Throughput vs Number of Threads for Mutexes with Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/s)"
set output 'mlab2b_4.png'
set key left top

plot \
     "< grep list-none-m,[0-9]*,1000,1, lab_2b_4.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'red', \
     "< grep list-none-m,[0-9]*,1000,4 lab_2b_4.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'orange',\
	 "< grep list-none-m,[0-9]*,1000,8 lab_2b_4.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue',\
     "< grep list-none-m,[0-9]*,1000,16 lab_2b_4.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'green'

# PLOT 5
set title "5. Throughput vs Number of Threads for Spin-locks with Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'mlab2b_5.png'
set key left top

plot \
     "< grep list-none-s,[0-9]*,1000,1, lab_2b_5.csv" using ($2):(1000000000/($7)) \
	title 'lists=1' with linespoints lc rgb 'red', \
     "< grep list-none-s,[0-9]*,1000,4 lab_2b_5.csv" using ($2):(1000000000/($7)) \
	title 'lists=4' with linespoints lc rgb 'orange',\
	 "< grep list-none-s,[0-9]*,1000,8 lab_2b_5.csv" using ($2):(1000000000/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue',\
     "< grep list-none-s,[0-9]*,1000,16 lab_2b_5.csv" using ($2):(1000000000/($7)) \
	title 'lists=16' with linespoints lc rgb 'green'




