#include "data_layer.h"
#include <math.h>

int send(char *port, char*file_name);

int build_data_packet(unsigned char *packet, unsigned char* data, int data_lenght, int seq_num);
int build_control_packet(unsigned char * packet, unsigned char* file_name, int file_size, unsigned char ctrl_field);

int read_data_packet(unsigned char *packet, int *seq_num, unsigned char *data);
int read_start_control_packet(unsigned char *packet, unsigned char *file_name, int *file_size);
