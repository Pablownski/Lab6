#!/usr/bin/env bash
set -euo pipefail

exe="./bin/p2_ring"
reps=5
items=200000

mkdir -p results

# Encabezado CSV
echo "P,C,Q,items,run,time_s,throughput" > results/p2_ring.csv

for P in 1 2 4; do
  for C in 1 2 4; do
    for Q in 256 1024 4096; do
      for r in $(seq 1 $reps); do
        out=$($exe $P $C $Q $items)
        t=$(echo "$out" | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
        th=$(echo "$out" | sed -n 's/.*throughput=\([0-9.]*\).*/\1/p')
        echo "$P,$C,$Q,$items,$r,$t,$th" >> results/p2_ring.csv
      done
    done
  done
done
