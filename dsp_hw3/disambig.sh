#!/bin/bash
cd ./srilm-1.5.10
for (( i=1; i<=10; i=i+1 ))
do
	./exec/disambig -text ../testdata/${i}\_seq.txt -map ../ZhuYin-Big5.map -lm bigram.lm -order 2 -keep-unk > ../result1/${i}.txt
done
