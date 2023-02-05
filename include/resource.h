#ifndef RESOURCE_H
#define RESOURCE_H
#include<stdbool.h>

/*simulate the resource avaliablity*/
extern bool *resource;

/*check the state of resource*/
void get_resources(int, int *);
void release_resources(int, int *);

#endif
