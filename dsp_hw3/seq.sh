#!/bin/bash
for (( i=1; i<=10; i=i+1 ))
do
	perl separator_big5.pl ./testdata/${i}.txt > ./testdata/${i}\_seq.txt
done
