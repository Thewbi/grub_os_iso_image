#!/bin/bash

make clean
rm ./testmain
gcc -c sort.c random.c
gcc -c sort_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./sort_test.o ./sort.o ./random.o -lcmocka
./testmain