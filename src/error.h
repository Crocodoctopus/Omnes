#pragma once

#define INVALID_INPUT 0
#define IO_ERROR 1
#define ALLOC_ERROR 2
#define ROM_FORMAT_ERROR 3
#define UNIMPLEMENTED 4
#define UNREACHABLE 5

static int error_counter = 0;

void inner_nlog(int, char*, char*, ...);
int inner_error(int, char*, int, char*, ...);
char* get_error_msg(int);
char get_error_id(int);

#define nlog(STRING, ...) inner_nlog(__LINE__, __FILE__, STRING, ##__VA_ARGS__)
#define error(ERRID, STRING, ...) inner_error(__LINE__, __FILE__, ERRID, STRING, ##__VA_ARGS__)
void nassert(int);