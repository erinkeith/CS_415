#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe SharedMem 1 
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o Sequential.o
#$ -e Sequential.e

./Sequential 2000 2000 2000
