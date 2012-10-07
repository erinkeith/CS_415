#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte 20
#$ -N S_Parallel
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o S_Parallel.o
#$ -e S_Parallel.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
export LD_LIBRARY_PATH=/usr/lib64/openmpi-1.5.3/lib:$LD_LIBRARY_PATH

for i in $(seq 2 2 6)
do
	echo "$i"
	for j in $(seq 1 1 10)
	do
		mpirun -np $i ./S_Parallel
	done
done
