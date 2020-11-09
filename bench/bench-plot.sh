#!/bin/sh
# Copyright (C) 2020 Reactive Markets Limited. All rights reserved.
set -eu -o pipefail

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

files=$(awk -vtmp="$tmp" '/#Name:/{name=$2; printf "%s", "\x27"tmp"/"name"\x27 using 4:1 with lines title \""name"\","} name{print $0 > tmp"/"name}' -)

gnuplot << EOF
set terminal svg noenhanced size 1280,300
set logscale x
set grid
set xlabel "Percentile"
set xtics ("0%%" 1.0, "90%%" 10.0, "99%%" 100.0, "99.9%%" 1000.0, "99.99%%" 10000.0, "99.999%%" 100000.0)
set ylabel "Latency (micros)"
set key top left
set title "Latency by Percentile Distribution"
show title
plot $files
EOF
