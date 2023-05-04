/*Non-Canonical Input Processing*/

// C library headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/types.h>
#include <sys/stat.h>

// Our Macros
#include "flags.h"
#include "utils.h"
#include "alarm.h"

//Layers
#include "app_layer.h"

//time tests
#include <time.h>

int main(int argc, char **argv)
{
  int fd, c, res;
  struct termios oldtio, newtio;
  // Allocate memory for read buffer, set size according to your needs
  char buf[255];

  char prefix[9];
  strncpy(prefix, argv[1], 9);

  clock_t start, finish;

  if (argc != 3 || (strcmp("/dev/ttyS", prefix)))
  {
    printf("Usage:\tnserial SerialPort FileName\n\tex: nserial /dev/ttyS10 teste.txt\n");
    exit(1);
  }
 
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        perror(argv[1]);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* not blocking */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");
  
  char *file_name = argv[2];
  FILE *file = file_open(file_name, "rb");
  if (file == NULL) exit(-1);

  int file_size = determine_file_size(file_name);

  printf("File name: %s\nFile Size: %d\n", file_name, file_size);

  if (llopen(fd, SENDER) == TRUE)
  {
    start = clock();
    printf("Set up completed\n");
  }
  else
  {
    printf("Failed to set up\n");
    exit(-1);
  }

  unsigned char *packet = malloc(MAX_PACKET_SIZE);
  int packet_length;

  packet_length = build_control_packet(packet, file_name, file_size, CTRL_START);
  if (packet_length < 0){
      printf("building control packet error\n"); 
      exit(-1); 
  }
  else {
    printf("control packet:");
    for (int i = 0; i < packet_length; i++)
      printf(" %x", packet[i]);
    printf("\n");
  }
  llwrite(fd, packet, packet_length);

  int seq_n = 0;
  int data_length = MAX_PACKET_SIZE - 4;  //Packets will have their maximum size, at the begining. (-4 is because of the C, N, L2, L1)
  char data[data_length];
  int remaining_file_size = file_size;

  
  while(TRUE){
    data_length = MAX_PACKET_SIZE - 4;

    if (remaining_file_size < data_length)
      data_length = remaining_file_size;

    if ((data_length = fread(data, 1, data_length, file)) <= 0)
      break; 
       
    if (build_data_packet(packet, data, data_length, seq_n) < 0){
      printf("building data packet (seq_n: %d) error\n", seq_n); 
      exit(-1); 
    }

    printf("sending packet with size = %d\n", read_data_packet(packet, &seq_n, data));
    printf("seq_n: %d\n", seq_n);
    
    //printf("Data_length: %d\n Packet_length: %d\n", data_length, data_length + 4);
    //for (int i = 0; i < data_length + 4; i++)
    //  printf("packet[%d] = %d\n", i, packet[i]);

    packet_length = data_length + 4;
    if (llwrite(fd, packet, packet_length) < 0) { 
      printf("llwrite error\n"); 
      return -1; 
    }

    remaining_file_size -= data_length;
    seq_n++;    
  }

  packet_length = build_control_packet(packet, file_name, file_size, CTRL_END); 
  llwrite(fd, packet, packet_length); 

  free(packet);

  llclose(fd, SENDER);
  finish = clock();
  double elapsed = (double)(finish - start)/CLOCKS_PER_SEC;
  printf("\n\nTime elapse : %f  \n\n", elapsed);
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
