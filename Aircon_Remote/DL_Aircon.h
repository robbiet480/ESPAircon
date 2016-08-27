#include <arduino.h>

#ifndef DL_Aircon_h
#define DL_Aircon_h

typedef struct {
  bool on;
  bool oscillate;
  bool timer;
  float timer_value;
  int speed;
  int wind;
} dl_aircon_msg_t;

unsigned long dl_assemble_msg(dl_aircon_msg_t* msg);
bool dl_decode_msg(dl_aircon_msg_t* msg);
void dl_print_msg(dl_aircon_msg_t *msg);
unsigned char bit_reverse( unsigned char x );


#endif
