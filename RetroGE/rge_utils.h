#ifndef __2D_RGE_UTILS
#define __2D_RGE_UTILS

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
//basic Fisher-Yates shuffle impl to allow us to randomize provided arrays via shuffling
void rge_shuffle_int_array(int* array, int size);
int rge_rand_int_from_range(int max);

//file functions 
//check if file exists (c standard method, but has caveats to use)
bool rge_file_exists(const char* filename);
#endif