#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 20
#$ -N S_Parallel7500
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o S_Parallel7500.o
#$ -e S_Parallel7500.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
#export LD_LIBRARY_PATH=/usr/lib64/openmpi-1.5.3/lib:$LD_LIBRARY_PATH
/usr/lib64/openmpi-1.5.3/bin/mpirun ./S_Parallel7500

#for i in $(seq 8 2 20)
#do
#	echo "$i"
#	for j in $(seq 1 1 5)
#	do
#		mpirun -np $i ./S_Parallel7500
#	done
#done
