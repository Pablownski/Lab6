#!/usr/bin/env bash
set -euo pipefail

exe="./bin/p1_counter"
reps=3        # repeticiones (ajústalo si quieres más)
iters=200000  # iteraciones por hilo

mkdir -p results

echo "impl,threads,iters,run,time_s,total,esperado" > results/p1_counter.csv

for T in 1 2 4 8 16; do
  for r in $(seq 1 $reps); do
    # Ejecuta el programa UNA vez y captura todas las variantes
    out=$($exe $T $iters)

    # Extrae cada implementación de la salida
    while read -r line; do
      impl=$(echo "$line" | awk '{print $1}')
      total=$(echo "$line" | sed -n 's/.*total=\([0-9]*\).*/\1/p')
      esperado=$(echo "$line" | sed -n 's/.*esperado=\([0-9]*\).*/\1/p')
      tiempo=$(echo "$line" | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
      echo "$impl,$T,$iters,$r,$tiempo,$total,$esperado" >> results/p1_counter.csv
    done <<< "$out"
  done
done