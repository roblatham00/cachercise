set terminal png
set output "updates-vs-xstream.png"

set ylabel "updates/sec"
set xlabel "processes"
set logscale x

plot "1xs.dat" using 1:10:11:12 w errorlines, \
    "4xs.dat" using 1:10:11:12 w errorlines, \
    "10xs.dat"  using 1:10:11:12 w errorlines, \
    "42xs.dat"  using 1:10:11:12 w errorlines
