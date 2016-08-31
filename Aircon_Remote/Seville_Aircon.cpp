#include "Seville_Aircon.h"

#ifndef SEVILLE_SERIAL
#define SEVILLE_SERIAL Serial
#endif

unsigned long seville_assemble_msg(seville_aircon_msg_t* msg){
  // 1 | msg->on | 2 | msg->timer | 3 | msg->oscillate | 4 | msg->speed
  unsigned long buf = 0x10203040;

  // if (msg->on) buf |= 0x1 << 21;

  // if (msg->speed) buf |= msg->speed;

  // if (msg->oscillate) buf |= msg->oscillate << 6;

  Serial.print("Buf ");
  Serial.print(buf);
  Serial.println();
}

void seville_print_msg(seville_aircon_msg_t* msg){
  SEVILLE_SERIAL.println("Settings:");

  if (msg->on){
    SEVILLE_SERIAL.println("ON  ");
  }else{
    SEVILLE_SERIAL.println("OFF ");
  }

  switch (msg->speed){
    case 3:
      SEVILLE_SERIAL.println("SPEED: Eco  ");
      break;
    case 0:
      SEVILLE_SERIAL.println("SPEED: Low  ");
      break;
    case 1:
      SEVILLE_SERIAL.println("SPEED: Mid  ");
      break;
    case 2:
      SEVILLE_SERIAL.println("SPEED: High ");
      break;
  }
}
