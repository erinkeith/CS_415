#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 8 
#$ -N DParallel
#$ -M keithhandschuh@hotmail.com
#$ -m es
#$ -o DParallel.o
#$ -e DParallel.e

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
/usr/lib64/openmpi-1.5.3/bin/mpirun ./DParallel
