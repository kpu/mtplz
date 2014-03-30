#!/usr/bin/gnuplot
set terminal tikz size 8.5,8
#set lmargin 9.5
set output 'model.tex'
set xlabel 'CPU seconds/sentence'
set ylabel 'Average model score'
set key bottom right
set ytics 0.5 format "%0.1f"
set xtics 1
plot [0:4.77332] [-29.5:-27.2] 'mtplz_probing' using 12:8 with lp title 'This Work', 'moses_probing' using 12:8 with lp title 'Moses'

set terminal tikz size 8,8
set ytics 1 format "%0.0f"
set output 'bleu.tex'
set ylabel 'Uncased BLEU'
plot [0:4.77332] [12.5:15.45] 'mtplz_probing' using 12:11 with lp title 'This Work', 'moses_probing' using 12:11 with lp title 'Moses'
