#ifndef _ALARM_H_
#define _ALARM_H_

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "flags.h"

void alarm_handler();

void set_up_alarm();

void set_alarm_time(unsigned int time);

void turn_alarm_off();

#endif
