#ifndef RANDOM_H
#define RANDOM_H

int is_rdrand_available();

unsigned int retrieve_rdrand();

// retrieve a pseudo random integer between 0 and USIGNED_INT_MAX
//unsigned int retrieve_pseudo_random(int, int);

unsigned long retrieve_pseudo_random(int x0);

#endif