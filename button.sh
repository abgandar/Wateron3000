#!/bin/sh

./ttycat -d "/dev/cu.wchusbserial1420" -s 38400 -e "!" -c "W" -f
