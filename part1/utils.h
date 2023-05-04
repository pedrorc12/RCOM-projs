#ifndef _UTILS_H_
#define _UTILS_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "flags.h"
#include "states.h"
#include "alarm.h"

extern int timeout;

int byte_stuffing(unsigned char *buffer, int lenght, unsigned char *processed_buffer);
int byte_clearing(unsigned char *buffer, int lenght, unsigned char *processed_buffer);
void print_buffer(char *buffer, int buffer_size);

FILE* file_open (char * filename, char * mode);

int determine_file_size(char *file_name);

#endif