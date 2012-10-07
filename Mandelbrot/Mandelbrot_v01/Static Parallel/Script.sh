#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 4
#$ -N S_Parallel
#$ -M keithhandschuh@hotmail.com
#$ -m es
#$ -o S_Parallel.o
#$ -e S_Parallel.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
/usr/lib64/openmpi-1.5.3/bin/mpirun ./S_Parallel
