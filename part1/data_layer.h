#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include "flags.h"
#include "utils.h"

struct frame
{
    enum {SUPERVISION, INFORMATION} type;
    unsigned char a;
    unsigned char c;
    unsigned char bcc1;
    unsigned char data_length;
    unsigned char *data;
    unsigned char bcc2;
};


int llopen(int port, short int actor);
int llwrite(int fd, unsigned char *buffer, int lenght);
int llread(int fd, unsigned char *buffer);
int llclose(int fd, short int actor);


/**
 * @brief Function that create a supervision frame used by the serial port ftp
 * @param frame
 * @param address_field
 * @param controlField
 * @return int 
 */
int create_supervision_frame(unsigned char* frame, unsigned char address_field, unsigned char controlField);


/**
 * @brief Create a information frame object
 * 
 * @param package_number
 * @param data
 * @param data_size
 * @param address_field 
 * @param information_frame 
 * @return int 
 */
int create_information_frame(unsigned short int package_number, unsigned char *data, int data_size, unsigned char address_field, unsigned char* frame);

/**
 * @brief Read the information frame
 * 
 * @param frame 
 * @param informations 
 * @return int 
 */
int parse_frame(int fd, struct frame *frame);

/**
 * @brief set up the serial port
 * 
 * @param port 
 * @return int, the fd 
*/
int set_up_termios(char *port);

/**
 * @brief Create a bcc2 byte
 * 
 * @param frame 
 * @param data_lenght 
 * @return unsigned char, the bcc2 octal
 */
unsigned char create_bcc2(unsigned char *frame, int data_lenght);

