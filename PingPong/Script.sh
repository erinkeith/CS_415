#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte-rr 2 
#$ -N PingPong
#$ -M keithhandschuh@hotmail.com
#$ -m es
#$ -o PingPong.o
#$ -e PingPong.e
#

export PATH=/usr/lib64/openmpi-1.5.3/bin:$PATH
/usr/lib64/openmpi-1.5.3/bin/mpirun ./a.out
