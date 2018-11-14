#!/bin/sh

DATE=`date "+ %d.%m.%Y %H.%M"`
echo "Waiting for data... "
./ttycat -d "/dev/cu.wchusbserial1420" -s 38400 -e "#EOR" -c "D" -f > "environment$DATE.dat" && \
echo "received: " && \
head -n3 "environment$DATE.dat" && \
echo "..." && \
echo "plotting..." && \
gnuplot -e "set term pdfcairo" -c environment.plot "environment$DATE.dat" > "environment$DATE.pdf" && \
open "environment$DATE.pdf" && \
echo "done."
