#ifndef RANDOM_H
#define RANDOM_H

int is_rdrand_available();

unsigned int retrieve_rdrand();

unsigned long retrieve_pseudo_random(int x0);

#endif