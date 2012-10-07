#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe SharedMem 1
#$ -N Sequential
#$ -M keithhandschuh@hotmail.com
#$ -m es
#$ -o Sequential.o
#$ -e Sequential.e

./Sequential
