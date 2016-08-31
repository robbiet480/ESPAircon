#include <arduino.h>

#ifndef Seville_Aircon_h
#define Seville_Aircon_h

typedef struct {
  bool on;
  bool oscillate;
  int speed;
} seville_aircon_msg_t;

#endif

