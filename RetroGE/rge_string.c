#include "rge_string.h"

rge_string_t* new_string(const char* new_string) {
    rge_string_t* ret = (rge_string_t*)malloc(sizeof(rge_string_t));
    ret->length = strlen(new_string) + 1; //+1 here for \0 character
    ret->text = (char*)malloc( sizeof ret->text * ret->length);
    memcpy(ret->text, new_string, ret->length);
    return ret;
}

void update_string(rge_string_t* update_string, const char* new_string) {
    update_string->length = strlen(new_string) + 1;  //+1 here for \0 character  

    // we allocate to prevent a memory leak if tmp is null
    char* tmp = (char*)realloc(update_string->text, sizeof update_string->text * update_string->length);

    if (tmp == NULL) {
        assert("Unable to realloc provided string!" && false);
    } else {
        update_string->text = tmp;
        memcpy(update_string->text, new_string, update_string->length);
    }
}

void dispose_string(rge_string_t* del_string) {
    //first free text
    free(del_string->text);
    
    //then free struct
    free(del_string);
}

//START of string functions
void substr(const char* source, int start_index, int length, char* destination, int dest_buffer_size) {
    //make sure length isn't great then dest
    if (length < dest_buffer_size) {
        snprintf(destination, length, "%s", source + start_index);
        //if it isn't copy everything up to strlen(destination)
    }
    else {
        snprintf(destination, dest_buffer_size, "%s", source + start_index);
    }
}

//returns character at offset in source string
//we use \0 to indicate null
char charAt(const char* source, size_t index) {
    char return_char = '\0';
    //snprintf will copy char+'\0', so to get the char we just
    //return index 0
    if (index < strlen(source)) {
        memcpy(&return_char, source + index, 1);
    }
    return return_char;
}

//returns offset in source character was located at
int indexOf(const char* source, int character) {
    char* ptr = strchr(source, character);
    if (ptr) {
        return ptr - source;
    }
    return -1;
}

//returns offset in source last instance character was located at
int lastIndexOf(const char* source, char character) {
    char* ptr = strrchr(source, character);
    if (ptr) {
        return ptr - source;
    }
    return -1;
}
//END of String functions


//tests for string functions
#ifdef DO_TESTS
void rge_string_test() {

    rge_string_t* str = new_string("This is a test string, wow, what a string!");
    printf("%d%s", str->length, str->text);

    for (int i = 0; i < 1200; i++) {
        char tmp[64];
        snprintf(tmp, 64, "%d", i);

        update_string(str, tmp);
        printf("%d%s\n", str->length, str->text);
    }

    dispose_string(str);
}
#endif 