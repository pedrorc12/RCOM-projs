/*Non-Canonical Input Processing*/

// C library headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Our Macros
#include "flags.h"
#include "utils.h"
#include "alarm.h"

//Layers
#include "app_layer.h"

int main(int argc, char **argv)
{

  char prefix[9];
  strncpy(prefix, argv[1], 9);


  if ((argc < 2) ||
      strcmp("/dev/ttyS", prefix))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
    exit(1);
  }

  int fd;
  if (fd = llopen(argv[1], SENDER) > 0)
  {
    printf("Set up completed\n");
    unsigned char hello[] = "hello";
    llwrite(fd, hello, sizeof(hello));
    llclose(fd, SENDER);
  }
  
  else
  {
    printf("Failed to set up\n");
  }

  return 0;
}