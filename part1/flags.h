#ifndef _MACROS_H_
#define _MACROS_H_

#define MODEMDEVICE     "/dev/ttyS1"
#define _POSIX_SOURCE   1

/**Actors*/
#define SENDER 0
#define RECEIVER 1

/**Protocol Flags*/
#define FLAG            0x7E
#define ESC             0x7D
#define END_SEND        0x03
#define END_REC         0x01
#define ESC             0x7D

/**Command Field*/
#define SET             0x03        /* SET command*/
#define DISC            0x0B        /* DISC command*/
#define UA              0x07        /* UA command*/ 
#define S(s)            (0x00 | ( s << 6 )) 
#define RR(r)           (0x05 | ( r << 7 ))
#define REJ(r)          (0x01 | ( r << 7 ))

/**Alarm Macros**/
#define ATTEMPTS        3   /* Attempts to read the receptor answers*/
#define TIMEOUT         3   /* Time in secondes to wait for the receptor answers*/

/** Application Macros **/
#define CTRL_DATA       0x01
#define CTRL_START      0x02
#define CTRL_END        0x03     
#define SENDER          0
#define RECEIVER        1
#define TYPE_FILE_SIZE  0x00
#define TYPE_FILE_NAME  0x01 

/**Helpful Macros**/
#define FALSE           0
#define TRUE            1
#define SWITCH(s)       !s
#define BIT(n)          (1 << n)
#define DELAY           0.1


#define BAUDRATE        B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define VTIME_VALUE     0
#define VMIN_VALUE      1

/**Config**/
#define MAX_FILE_SIZE        16000
#define MAX_FILENAME_SIZE    30
#define MAX_FRAME_SIZE       512
#define MAX_PACKET_SIZE      30
#define S_FRAME_SIZE         5

#endif
