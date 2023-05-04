/*Non-Canonical Input Processing*/

// C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flags.h"
#include "states.h"
#include "utils.h"
#include "alarm.h"

//Layers
#include "app_layer.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int main(int argc, char **argv)
{

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
       (strcmp("/dev/ttyS11", argv[1]) != 0) &&
       (strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
    exit(1);
  }

  int fd;
  if (fd = llopen(argv[1], RECEIVER) > 0)
  {
    printf("Set up completed\n");
    unsigned char buffer[MAX_FRAME_SIZE];
    int size = llread(fd, buffer);
    printf("Received the message: \"%s\"\n", buffer);
    llclose(fd, RECEIVER);
  }
  else
  {
    printf("Failed to set up\n");
  }

  return 0;
}