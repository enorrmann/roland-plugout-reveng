#!/bin/sh
# runq.sh NAME f1 f2 f3 f4: poke four probe files (live/*.txt, without .txt) into d2a d2b d3a d3b, record,
# print "file result(gain-1)" for both slaves
cd "$(dirname "$0")/../../../.." || exit 1
N=$1; shift
python3 re/dsplive.py poke d2a=live/$1.txt d2b=live/$2.txt d3a=live/$3.txt d3b=live/$4.txt >/dev/null || exit 1
python3 re/dsplive.py recall $N 2>&1 | grep gain | sort -k2,2 -s | LC_ALL=C awk '{printf "%-14s %+.3f\n",$3,$5-1}' | paste - -
