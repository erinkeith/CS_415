#!/bin/bash

#$ -cwd
#$ -pe SharedMem 1
#$ -N Hello_World
#$ -M keithhandschuh@hotmail.com
#$ -m bes

make
./HelloWorld
