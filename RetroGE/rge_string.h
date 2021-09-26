#ifndef __2D_RGE_STRING
#define __2D_RGE_STRING

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

//represents a heap allocated string, init using new_string, destroy using dispose_string, update using update_string
typedef struct rge_string {
	char* text;
	size_t length;
} rge_string_t;

//init a string, returns pointer to new string
rge_string_t* new_string(const char* new_string);
//update an existing string (realloc)
void update_string(rge_string_t* update_string, const char* new_string);
//frees a rge_string_t
void dispose_string(rge_string_t* new_string);

//get the substring starting at start_index and ending at start_index + length
void substr(const char* source, int start_index, int length, char* destination, int dest_buffer_size);
//get character at index, returns \0 if null
char charAt(const char* source, size_t index);
//returns offset in source character was located at, returns -1 if not found
int indexOf(const char* source, int character);
//returns offset in source last instance character was located at, returns -1 if not found
int lastIndexOf(const char* source, char character);
#endif