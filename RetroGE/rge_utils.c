#include "rge_utils.h"

//return a random integer from 0 to max (inclusive)
int rge_rand_int_from_range(int max) {
    return rand() % (max+1);
}

//basic Fisher-Yates shuffle impl to allow us to randomize provided arrays via shuffling
void rge_shuffle_int_array(int* array, int size) {
    int i, j, tmp;

    for (i = size - 1; i > 0; i--) {
        j = rand() % size;
        tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
    }
}

bool rge_file_exists(const char* filename) {
    FILE* file;

    if ((file = fopen(filename, "r")) != NULL) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}