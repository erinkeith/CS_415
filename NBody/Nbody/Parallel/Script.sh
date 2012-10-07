#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 51
#$ -N Parallel
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o parallel.o
#$ -e parallel.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
export LD_LIBRARY_PATH=/usr/lib64/openmpi-1.5.3/lib:$LD_LIBRARY_PATH

#for i in $(seq 1 1 5)
#do
	mpirun ./Parallel
#done
