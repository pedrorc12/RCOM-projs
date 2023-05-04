#include "app_layer.h"

struct applicationLayer {
    int fileDescriptor; /*serial port descriptor*/ 
    int status; /*TRANSMITER | RECEIVER*/
} app_layer;


int send(char *port, char*file_name){
  FILE *fp = file_open(file_name, "r+w");

  app_layer.status = SENDER;
 
}

int build_data_packet(unsigned char *packet, unsigned char* data, int data_lenght, int seq_n)
{

  packet[0] = CTRL_DATA;
  packet[1] = seq_n % 256;
  packet[2] = data_lenght / 256;
  packet[3] = data_lenght % 256;

  memcpy(&packet[4], data, data_lenght);
  return 0;
}

int build_control_packet(unsigned char * packet, unsigned char* file_name, int file_size, unsigned char ctrl_field) {
  int pos = 0;

  packet[pos++] = ctrl_field;
  packet[pos++] = TYPE_FILE_NAME;
  packet[pos++] = strlen(file_name);

  memcpy(&packet[pos], file_name, strlen(file_name));

  pos += strlen(file_name);

  packet[pos++] = TYPE_FILE_SIZE;

  int initial_pos = ++pos;
  int file_size_len = 0;
  while (file_size > 0)
    {
        //Shifting the bytes to the right, to write new more significant byte
        for (int i = pos; i > initial_pos; i--)
            packet[i] = packet[i - 1];

        packet[initial_pos] = (unsigned char) file_size % 256; //Writes the most significant byte so far
        file_size /= 256;

        pos++;
        file_size_len++;
    }
    packet[initial_pos - 1] = file_size_len;

  return pos;
}

int read_data_packet(unsigned char *packet, int *seq_num, unsigned char *data) {
  
  *seq_num = packet[1];
  int data_size =  packet[2] * 256 + packet[3];
  printf("%d = %x + %x", data_size, packet[2], packet[3]);
  memcpy(data, &packet[4], data_size);

  return data_size;
}

int read_start_control_packet(unsigned char *packet, unsigned char *file_name, int *file_size) {

  int info_read = 0;
  int pos = 1;
  while (info_read < 2) {
    
    if (packet[pos] == TYPE_FILE_NAME) {
      pos++;
      int file_name_len = packet[pos++];
      for (int i = 0; i < file_name_len; i++) {
        file_name[i] = packet[pos++];
      }
      file_name[file_name_len] = '\0';
      info_read++;
    }
    else if (packet[pos] == TYPE_FILE_SIZE) {
      pos++;
      int file_size_len = packet[pos++];
      *file_size = 0;
      for (int i = 0; i < file_size_len; i++) {
        *file_size = (*file_size)*256 + packet[pos + i];
      }
      pos += file_size_len;
      info_read++;
    }
    else
      break;
  }
  return pos;
}
