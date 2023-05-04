#include "utils.h"


int byte_stuffing(unsigned char *buffer, int length, unsigned char *processed_buffer)
{
    int j = 0;
    for (int i = 0; i < length; i++, j++)
    {
        if (buffer[i] == FLAG)
        {
            processed_buffer[j] = ESC;
            j++;
            processed_buffer[j] = buffer[i] ^ 0x20;
        }
        else if (buffer[i] == ESC)
        {
            processed_buffer[j] = ESC;
            j++;
            processed_buffer[j] = buffer[i] ^ 0x20;
        }
        else
        {
            processed_buffer[j] = buffer[i];
        }
    }
    //DEBUG
    //printf("frame stuffed size: %d\n", j);
    return j + 1;
}

int byte_clearing(unsigned char *buffer, int length, unsigned char *processed_buffer)
{
    int j = 0;
    for (int i = 0; i < length; i++, j++)
    {
        if (buffer[i] == ESC)
        {
            i++;
            processed_buffer[j] = buffer[i] ^ 0x20;
        }
        else
        {
            processed_buffer[j] = buffer[i];
        }
    }
    //DEBUG
    //printf("frame cleard size: %d\n", j);
    return j;
}

FILE* file_open (char * filename, char * mode){
  FILE *fptr;

  fptr = fopen(filename, mode);

  if (fptr == NULL){
      printf("Error opening file %s,", filename);
      return NULL;
  }

  return fptr; 
}

int determine_file_size(char *file_name){
    
    struct stat file_info;
    if(stat(file_name, &file_info) == -1){
        perror("Error getting file size");
        return -1;
    }
    return file_info.st_size;
}

void print_buffer(char *buffer, int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        printf(" %x", buffer[i]);
    }
    printf("\n");
}