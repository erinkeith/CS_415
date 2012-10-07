#!/bin/bash


#$ -q BrainLab.owner

#$ -cwd
#$ -pe orte 1 

#$ -M nitishn@gmail.com

#$ -m s

#$ -o pa3.o

#$ -e pa3.e

./pa3 

100 100 8000 
