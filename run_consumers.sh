#!/bin/bash
# $1 number of iterations

./consumer $1 1 100 &
./consumer $1 2 101 &
./consumer $1 3 102 &
./consumer $1 4 103 &
./consumer $1 5 104 &
