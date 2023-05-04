/*Non-Canonical Input Processing*/

// C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/types.h>
#include <sys/stat.h>

#include "flags.h"
#include "states.h"
#include "utils.h"
#include "alarm.h"
#include <time.h>

//Layers
#include "app_layer.h"

int main(int argc, char **argv)
{
  int fd, c, res;
  struct termios oldtio, newtio; // Create new termios struct
  char buf[255];

  char prefix[9];
  strncpy(prefix, argv[1], 9);

  /*if (argc != 2 || (strcmp("/dev/ttyS0", prefix)))
  {
    printf("Usage:\tnserial SerialPort FileName\n\tex: nserial /dev/ttyS10 test.txt\n");
    exit(1);
  }*/
 

  char *file_name = (char *) malloc(MAX_FILENAME_SIZE);
  int file_size;
  fd = set_up_termios(argv[1]);
  if (llopen(fd, RECEIVER) == TRUE)
  {
    printf("Set up completed\n");
  }
  else
  {
    printf("Failed to set up\n");
    exit(-1);
  }

  char *packet = (char *) malloc(MAX_PACKET_SIZE);
  int packet_length = MAX_PACKET_SIZE;

  while (TRUE) {
    packet_length = llread(fd, packet);
    if (packet_length <= 0) continue;
    if (packet[0] == CTRL_START) {
      printf("Start packet received");
      read_start_control_packet(packet, file_name, &file_size);
      printf("File name: %s\nFile size: %d\n", file_name, file_size);
      break;
    }
  } 

  FILE *file = fopen(file_name, "wb");
  if (file == NULL){
    printf("Error creating the file\n");
    exit(-1);
  }

  char *data = (char *) malloc(MAX_PACKET_SIZE - 4);
  int data_length = MAX_PACKET_SIZE - 4;
  int seq_num;
  while (TRUE) { 
    packet_length = llread(fd, packet);
    
    //TEST
    //sleep(1);
    if (packet_length <= 0) continue;

    if (packet[0] == CTRL_DATA) {
      data_length = read_data_packet(packet, &seq_num, data);

      //for(int i = 0; i < data_length; i++)
      //  printf("data[%d] = %x\n", i, data[i]);
      
      fwrite(data, 1, data_length, file);
      printf("Read seq_num %d", seq_num);
    }

    else if (packet[0] == CTRL_END) break;
  }

  free(data);
  free(file_name);
  free(packet);

  llclose(fd, RECEIVER);

  // Save tty settings
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
