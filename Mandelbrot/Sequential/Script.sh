#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte 1
#$ -N Sequential
#$ -M keithhandschuh@hotmail.com
#$ -m es
#$ -o Sequential.o
#$ -e Sequential.e

./Sequential > panda.PGM
