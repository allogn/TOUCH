set terminal png
set output "mesh_hilbert_values.png"
set logscale y
set xlabel "Number of objects sharing the same Hilbert value"
set ylabel "Number of Hilbert values"
plot "mesh_hilbert_values.data" with linespoints notitle
