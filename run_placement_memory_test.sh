#!/bin/bash

make clean
rm ./testmain
gcc -c sort.c random.c placement_memory.c common.c
gcc -c placement_memory_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./common.o ./sort.o ./random.o ./placement_memory.o -lcmocka
./testmain