#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 20 
#$ -N DParallel10000
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o DParallel10000.o
#$ -e DParallel10000.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
export LD_LIBRARY_PATH=/usr/lib64/openmpi-1.5.3/lib:$LD_LIBRARY_PATH
#/usr/lib64/openmpi-1.5.3/bin/mpirun ./DParallel

for i in $(seq 16 2 20)
do
	echo "$i"
	for j in $(seq 1 1 5)
	do
		mpirun -np $i ./DParallel10000
	done
done
