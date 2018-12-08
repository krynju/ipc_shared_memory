#!/bin/bash
# $1 - number of producers
# $2 - number of iterations
for i in `seq 1 $1`;
	do
		./producer_mul $2 $i 100 101 102 103 104 &
	done 
