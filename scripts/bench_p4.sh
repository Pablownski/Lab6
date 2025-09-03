#!/usr/bin/env bash
set -euo pipefail

exe="./bin/p4_deadlock_fixed"

mkdir -p results

echo "=== Práctica 4 - Deadlock corregido ==="
$exe
