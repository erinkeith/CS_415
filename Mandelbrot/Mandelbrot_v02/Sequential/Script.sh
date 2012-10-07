#!/bin/bash

#$ -q BrainLab.owner
#$ -cwd
#$ -pe orte 1
#$ -N Sequential
#$ -M keithhandschuh@hotmail.com
#$ -m s
#$ -o Sequential.o
#$ -e Sequential.e

./Sequential
