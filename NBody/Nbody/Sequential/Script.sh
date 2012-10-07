#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe SharedMem 1 
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o Sequential.o
#$ -e Sequential.e

for i in $(seq 1 1 5)
do
	./sequential 10
done
