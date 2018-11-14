# call as "gnuplot -c environment.plot <DATAFILE>"
set xlabel "Time (hours)"
set ylabel "Temperature (°C)"
set y2label "Humidity (%)"
set y2tics
set ytics nomirror
set xtics -48, 12
set mxtics 4
set ytics 0, 10
set mytics 2
set grid
set key top center horizontal outside
stats ARG1 index 0 nooutput
plot [-48:0] [0:40] [:] [0:100] \
	10 with lines axes x1y1 dt 3 lc 0 lw 0.5 not, \
	20 with lines axes x1y1 dt 3 lc 0 lw 0.5 not, \
	30 with lines axes x1y1 dt 3 lc 0 lw 0.5 not, \
	ARG1 using (($0-STATS_records)*4/60):1 index 1 with lines axes x1y2 lc 6 lw 2 title "Humidity (Indoor)", \
	ARG1 using (($0-STATS_records)*4/60):1 index 0 with lines axes x1y1 lc 7 lw 2 title "Temperature (Indoor)", \
	ARG1 using (($0-STATS_records)*4/60):1 index 3 with lines axes x1y2 lc 3 lw 2 title "Humidity (Outdoor)", \
	ARG1 using (($0-STATS_records)*4/60):1 index 2 with lines axes x1y1 lc 4 lw 2 title "Temperature (Outdoor)"
