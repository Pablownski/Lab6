#!/usr/bin/env bash
set -euo pipefail

exe="./bin/p3_rw"
reps=3
ops=100000

mkdir -p results

echo "threads,ops,write_ratio,mode,run,time_s,throughput" > results/p3_rw.csv

for T in 1 2 4 8; do
  for WR in 0.1 0.3 0.5; do   # 90/10, 70/30, 50/50
    for mode in 0 1; do       # 0 = mutex, 1 = rwlock
      for r in $(seq 1 $reps); do
        out=$($exe $T $ops $WR $mode)
        t=$(echo "$out" | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
        th=$(echo "$out" | sed -n 's/.*throughput=\([0-9.]*\).*/\1/p')
        echo "$T,$ops,$WR,$mode,$r,$t,$th" >> results/p3_rw.csv
      done
    done
  done
done
