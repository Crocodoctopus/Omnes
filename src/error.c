#include <assert.h>

#include "error.h"

#ifdef WASM32

void inner_nlog(int line, char* file, char* string, ...) {
}

int inner_error(int line, char* file, int err, char* string, ...) {
    error_counter += 1;
    return error_counter;
}

// TODO: map error ID to associated error message.
char* get_error_msg(int errorid) {
    assert(0);
    return 0;
}

// TODO: map error ID to associated error type.
char get_error_type(int errorid) {
    assert(0);
    return 0;
}

#endif

#ifdef X86
#include <stdio.h>
#include <stdarg.h>

void inner_nlog(int line, char* file, char* string, ...) {
    va_list args;
    va_start(args, string);

    printf("[%s:%i] => ", file, line);
    vprintf(string, args);
    printf("\n");
    fflush(stdout);

    va_end(args);
}

int inner_error(int line, char* file, int err, char* string, ...) {
    va_list args;
    va_start(args, string);

    printf("ERROR [%s:%i] => ", file, line);
    vprintf(string, args);
    printf("\n");
    fflush(stdout);

    va_end(args);
    
    error_counter += 1;
    return error_counter;
}

// TODO: map error ID to associated error message.
char* get_error_msg(int errorid) {
    assert(0);
    return 0;
}

// TODO: map error ID to associated error type.
char get_error_type(int errorid) {
    assert(0);
    return 0;
}

#endif