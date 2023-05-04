#include "alarm.h"


int timeout;

void set_up_alarm(){
   (void)signal(SIGALRM, alarm_handler); // instala  rotina que atende interrupcao
}

void alarm_handler() // atende alarme
{
   timeout = TRUE;
   return;
}

void set_alarm_time(unsigned int time){
   timeout = FALSE;
   alarm(time);
}

void turn_alarm_off(){
   alarm(0);
}

/* int main(int argc, char **argv)
{  
   set_up_alarm();

   while (conta < 4)
   {
      if (timeout)
      {
         alarm(3); // activa alarme de 3s
         timeout = 0;
      }
   }
   printf("Vou terminar.\n");
} */
