#!/bin/sh

id=$1
outfile=../../../../../../draft/tables/tables_$id\_$2.dat
nodes=$2
opt=../../../$3/data/opt_mp50.dat
heu1=../../../$3/data/heu_ml1.dat
heu2=../../../$3/data/heu_ml2.dat
heu4=../../../$3/data/heu_ml4.dat
heu8=../../../$3/data/heu_ml8.dat
heu16=../../../$3/data/heu_ml16.dat
heu32=../../../$3/data/heu_ml32.dat

> $outfile

echo "\\\begin{table}[!htbp]" >> $outfile
echo "   \\\centering" >> $outfile
echo "    \\\begin{tabular}{ | l | p{0.75cm} | p{1.0cm} | p{1.0cm} | p{0.75cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} |}"  >> $outfile
echo "    \hline" >> $outfile
echo "    \multirow{3}{*}{Type} & \multicolumn{3}{ c| }{Performance} & \multicolumn{5}{ c| }{Num. of Paths} \\\\\\\\ \\\cline{2-9}" >> $outfile
echo "    & Data (MB) & Time ($\mu\$s) & BW (MB/s)& \multirow{2}{1cm}{Total Paths} & \multicolumn{4}{ c| }{Per Job} \\\\\\\\ \\\cline{6-9}" >> $outfile
echo "    & & & & & Max & Min & Avg & Med \\\\\\\\ \hline" >> $outfile

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    OPT & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu1 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu2 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu4 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu8 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu16 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu32 | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    MPI & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> $outfile

echo "    \end{tabular}" >> $outfile
echo "    \\\caption{Performance with number of paths in $nodes nodes experiments}" >> $outfile
echo "    \label{table:$nodes_perf}" >> $outfile
echo "\end{table}" >> $outfile

echo "" >> $outfile
echo "" >> $outfile

echo "\\\begin{table}[!htbp]" >> $outfile
echo "    \\\centering" >> $outfile
echo "    \\\begin{tabular}{ | l | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.75cm} | p{0.5cm} | p{1.0cm} | p{0.5cm} |}" >> $outfile
echo "    \hline" >> $outfile
echo "    \multirow{3}{*}{Type} & \multicolumn{4}{ c| }{Num. of Hops} & \multicolumn{4}{ c| }{Num. of Copies} \\\\\\\\ \\\cline{2-9}" >> $outfile
echo "    & \multicolumn{4}{ c| }{Per Path} & \multicolumn{4}{ c| }{Per Node} \\\\\\\\ \\\cline{2-9}" >> $outfile
echo "    & Max & Min & Avg & Med &Max & Min & Avg & Med \\\\\\\\ \hline" >> $outfile

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    OPT &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu1 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu2 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu4 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu8 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu16 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu32 | awk '{print "    HEU &  " $29 " & " $30 " & " $31 " & " $32 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> $outfile
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    MPI &  " $29 " & " $30 " & " $31 " & " $32 " & "  " & " " & " " & " " \\\\ \\hline"}'  >> $outfile

echo "    \end{tabular}" >> $outfile
echo "    \\\caption{Number of hops and copies in $nodes nodes experiments}" >> $outfile
echo "    \label{table:$nodes_hopcopy}" >> $outfile
echo "\end{table}" >> $outfile

echo "" >> $outfile
echo "" >> $outfile

echo "\\\begin{table}[!htbp]" >> $outfile
echo "    \\\centering" >> $outfile
echo "    \\\begin{tabular}{ | l | p{1.5cm} | p{1.5cm} | p{1.5cm} | p{1.5cm} | p{0.75cm} |  p{0.5cm} | p{1cm} | p{0.75cm} |}" >> $outfile
echo "    \hline" >> $outfile
echo "    \multirow{3}{*}{Type} & \multicolumn{4}{ c| }{Hopbytes} & \multicolumn{4}{ c| }{Num. of Copies} \\\\\\\\ \\\cline{2-9}" >> $outfile
echo "    & \multicolumn{4}{ c| }{Per Path}  & \multicolumn{4}{ c| }{Per Path} \\\\\\\\ \\\cline{3-9}" >> $outfile
echo "    & Max & Min & Avg & Med & Max & Min & Avg & Med \\\\\\\\ \hline" >> $outfile

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    OPT &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu1 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu2 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu4 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu8 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu16 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu32 | awk '{print "    HEU &  " $19 " & " $20 " & " int($21) " & " $22 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> $outfile
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    MPI &  " $19 " & " $20 " & " int($21) " & " $22 " & "  " & " " & " " & " " \\\\ \\hline"}'  >> $outfile

echo "    \end{tabular}" >> $outfile
echo "    \\\caption{Number of hopbytes and copies per path in $nodes nodes experiments}" >> $outfile
echo "    \label{table:$nodes_hopbyte}" >> $outfile
echo "\end{table}" >> $outfile

echo "" >> $outfile
echo "" >> $outfile

echo "\\\begin{table}[!htbp]" >> $outfile
echo "    \\\centering" >> $outfile
echo "    \\\begin{tabular}{ | l | p{0.75cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{1.5cm} | p{1.5cm} | p{1.25cm} | p{1.5cm} |}" >> $outfile
echo "    \hline" >> $outfile
echo "    \multirow{2}{*}{Type} & \multicolumn{5}{ c| }{Load (Num of Paths) per Link} & \multicolumn{4}{ c| }{Actual Load (Data Amount) per Link} \\\\\\\\ \\\cline{2-10}" >> $outfile
echo "    & Total & Max & Min & Avg & Med & Max & Min & Avg & Med \\\\\\\\ \hline" >> $outfile

grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    OPT &  "  $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu1 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu2 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu4 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu8 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu16 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "O  $id OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" $heu32 | awk '{print "    HEU &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile
grep "M  $id MPI_Alltoallv  msg = 8388608 chunk = 65536" $opt | awk '{print "    MPI &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> $outfile

echo "    \end{tabular}" >> $outfile
echo "    \\\caption{Load: number of paths and actual total amount of data over physical links in $nodes nodes experiments}" >> $outfile
echo "    \label{table:$nodes_load}" >> $outfile
echo "\end{table}" >> $outfile
