#!/bin/bash

make clean
rm ./testmain
gcc -c bitmap.c
gcc -c common.c
gcc -c bitmap_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./bitmap_test.o ./bitmap.o ./common.o -lcmocka
./testmain