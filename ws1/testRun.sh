#!/bin/bash

i="0"

printf "TEST_NUM, MATCHES, GENERATION\n" >> out.txt

make clean
make



_now=$(date +"%m_%d_%Y_%s")

while [ $i -lt 5 ]
do

  printf "$i, " >> out.txt
  ./ws1
  i=$[$i+1]
  sleep 3

  _history_now=${_now}_plots_${i}
  _file2="$_history_now.csv"
  cp ./history.csv ./results/$_file2
done


_file="$_now.csv"

cp ./out.txt ./results/$_file
rm out.txt
touch out.txt

{ head -n1 ./results/*plots_1.csv; for f in ./results/*plots*.csv; do printf "\n"; tail -n+2 "$f"; done; } > ./results/merged_$_now.csv

rm ./results/*plots*.csv
