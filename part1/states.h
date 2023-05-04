#ifndef _STATES_H_
#define _STATES_H_

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    DATA_RCV,
    BCC2_OK,
    STOP
} genericState;

#endif
