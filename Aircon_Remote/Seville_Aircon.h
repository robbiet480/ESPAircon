#include <arduino.h>

#ifndef Seville_Aircon_h
#define Seville_Aircon_h

typedef struct {
  bool on;
  bool oscillate;
  int speed;
} seville_aircon_msg_t;

unsigned long seville_assemble_msg(seville_aircon_msg_t* msg);
// bool seville_decode_msg(seville_aircon_msg_t* msg);
void seville_print_msg(seville_aircon_msg_t *msg);


#endif
