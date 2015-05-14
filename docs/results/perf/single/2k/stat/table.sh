> tables.dat

echo "\begin{table}[h]" >> tables.dat
echo "   \centering" >> tables.dat
echo "    \begin{tabular}{ | l | p{0.75cm} | p{1.0cm} | p{1.0cm} | p{0.75cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} |}"  >> tables.dat
echo "    \hline" >> tables.dat
echo "    \multirow{3}{*}{Type} & \multicolumn{3}{ c| }{Performance} & \multicolumn{5}{ c| }{Num. of Paths} \\\\ \cline{2-9}" >> tables.dat
echo "    & Data (MB) & Time ($\mu\$s) & BW (MB/s)& \multirow{2}{1cm}{Total Paths} & \multicolumn{4}{ c| }{Per Job} \\\\ \cline{6-9}" >> tables.dat
echo "    & & & & & Max & Min & Avg & Med \\\\ \hline" >> tables.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    OPT & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> tables.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" heu_ml4.dat | awk '{print "    HEU & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> tables.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    MPI & " $10 "  & " int($11) " & " int($12) " & " $13 " & " $14 " & " $15 " & " $16 " & " $17 " \\\\ \\hline"}' >> tables.dat

echo "    \end{tabular}" >> tables.dat
echo "    \caption{Performance with number of paths in 2048 nodes experiments}" >> tables.dat
echo "    \label{table:2048_perf}" >> tables.dat
echo "\end{table}" >> tables.dat

echo "" >> tables.dat
echo "" >> tables.dat

echo "\begin{table}[h]" >> tables.dat
echo "    \centering" >> tables.dat
echo "    \begin{tabular}{ | l | p{0.75cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{1.0cm} | p{0.75cm} | p{0.5cm} | p{1.0cm} | p{0.5cm} |}" >> tables.dat
echo "    \hline" >> tables.dat
echo "    \multirow{3}{*}{Type} & \multicolumn{5}{ c| }{Num. of Hops} & \multicolumn{5}{ c| }{Num. of Copies} \\\\ \cline{2-11}" >> tables.dat
echo "    & \multirow{2}{1cm}{Total hops} & \multicolumn{4}{ c| }{Per Path} & \multirow{2}{1cm}{Total copies} & \multicolumn{4}{ c| }{Per Node} \\\\ \cline{3-6} \cline{8-11}" >> tables.dat
echo "    & & Max & Min & Avg & Med & & Max & Min & Avg & Med \\\\ \hline" >> tables.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    OPT &  " $28 " & " $29 " & " $30 " & " $31 " & " $32 " & " $33 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> tables.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" heu_ml4.dat | awk '{print "    HEU &  " $28 " & " $29 " & " $30 " & " $31 " & " $32 " & " $33 " & " $34 " & " $35 " & " $36 " & " $37 " \\\\ \\hline"}'  >> tables.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    MPI &  " $28 " & " $29 " & " $30 " & " $31 " & " $32 " & "  " & "  " & " " & " " & " " \\\\ \\hline"}'  >> tables.dat

echo "    \end{tabular}" >> tables.dat
echo "    \caption{Number of hops and copies in 2048 nodes experiments}" >> tables.dat
echo "    \label{table:2048_hopcopy}" >> tables.dat
echo "\end{table}" >> tables.dat

echo "" >> tables.dat
echo "" >> tables.dat

echo "\begin{table}[h]" >> tables.dat
echo "    \centering" >> tables.dat
echo "    \begin{tabular}{ | l | p{2cm} | p{1.5cm} | p{1.5cm} | p{1.5cm} | p{1.5cm} | p{1.0cm} | p{0.75cm} | p{0.5cm} | p{1cm} | p{0.75cm} |}" >> tables.dat
echo "    \hline" >> tables.dat
echo "    \multirow{3}{*}{Type} & \multicolumn{5}{ c| }{Hopbytes} & \multicolumn{5}{ c| }{Num. of Copies} \\\\ \cline{2-11}" >> tables.dat
echo "    & \multirow{2}{1cm}{Total hopbytes} & \multicolumn{4}{ c| }{Per Path} & \multirow{2}{1cm}{Total copies} & \multicolumn{4}{ c| }{Per Path} \\\\ \cline{3-6} \cline{8-11}" >> tables.dat
echo "    & & Max & Min & Avg & Med & & Max & Min & Avg & Med \\\\ \hline" >> tables.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    OPT &  " $18 " & " $19 " & " $20 " & " int($21) " & " $22 " & " $23 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> tables.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" heu_ml4.dat | awk '{print "    HEU &  " $18 " & " $19 " & " $20 " & " int($21) " & " $22 " & " $23 " & " $24 " & " $25 " & " $26 " & " $27 " \\\\ \\hline"}'  >> tables.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    MPI &  " $18 " & " $19 " & " $20 " & " int($21) " & " $22 " & "  " & "  " & " " & " " & " " \\\\ \\hline"}'  >> tables.dat

echo "    \end{tabular}" >> tables.dat
echo "    \caption{Number of hopbytes and copies per path in 2048 nodes experiments}" >> tables.dat
echo "    \label{table:2048_hopbyte}" >> tables.dat
echo "\end{table}" >> tables.dat

o "" >> tables.dat
echo "" >> tables.dat

echo "\begin{table}[h]" >> tables.dat
echo "    \centering" >> tables.dat
echo "    \begin{tabular}{ | l | p{0.75cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{0.5cm} | p{1.5cm} | p{1.5cm} | p{1.25cm} | p{1.5cm} |}" >> tables.dat
echo "    \hline" >> tables.dat
echo "    \multirow{2}{*}{Type} & \multicolumn{5}{ c| }{Load (Num of Paths) per Link} & \multicolumn{4}{ c| }{Actual Load (Data Amount) per Link} \\\\ \cline{2-10}" >> tables.dat
echo "    & Total & Max & Min & Avg & Med & Max & Min & Avg & Med \\\\ \hline" >> tables.dat

grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    OPT &  "  $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> tables.dat
grep "O  87 OPTIQ_Alltoallv  msg = 8388608 chunk = 65536" heu_ml4.dat | awk '{print "    HEUT &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> tables.dat
grep "M  87 MPI_Alltoallv  msg = 8388608 chunk = 65536" opt_mp50.dat | awk '{print "    MPI &  " $13 " & " $38 " & " $39 " & " $40 " & " $41 " & " $42 " & " $43 " & " int($44) " & " $45 " \\\\ \\hline"}'  >> tables.dat

echo "    \end{tabular}" >> tables.dat
echo "    \caption{Load: number of paths and actual total amount of data over physical links in 2048 nodes experiments}" >> tables.dat
echo "    \label{table:2048_load}" >> tables.dat
echo "\end{table}" >> tables.dat
