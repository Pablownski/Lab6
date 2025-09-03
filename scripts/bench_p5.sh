#!/usr/bin/env bash
set -euo pipefail

exe="./bin/p5_pipeline"
reps=3
items=50000

mkdir -p results

echo "items,run,stage,time_s" > results/p5_pipeline.csv

for n in 50000 100000 200000; do
  for r in $(seq 1 $reps); do
    out=$($exe $n)
    echo "$out" | grep "Stage" | while read line; do
      stage=$(echo $line | awk '{print $1}')
      t=$(echo $line | sed -n 's/.*en \([0-9.]*\) s.*/\1/p')
      echo "$n,$r,$stage,$t" >> results/p5_pipeline.csv
    done
  done
done
