#include "DL_Aircon.h"

#ifndef DL_SERIAL
#define DL_SERIAL Serial
#endif

/*
 * DeLonghi PAC N81 IR Commands
 *
 * 32 Bit Value
 * NEC Encoding
 *
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |   0x01    |   0x02    |  |Lo|Md|Hi|Ac|  |Dh|Bl|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Timer     |  |CF|T |ON|    Temperature        |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * 0x01 and 0x02 is a fixed prefix
 * Lo / Md / Hi are bits for the fan setting
 * Cl / Dh / Bl are mode switches
 *    Ac  Airconditioning
 *    Dh  Dehumidify
 *    Bl  Blow (Fan Mode)
 * Timer is the set value for timer control.
 *    Set to 1 even if not active via the bit
 * CF   Celsius / Fahrenheit Setting. Low C, High F
 * T    Timer Low Off, High On
 * On   On/Off Switch
 *
 * Temperature    8-Bit Temperature Value
 *
 * 0-16 means 16-32°C
 * 61-89 means 61-89°F
 *
 * Both the temperature and timer have their bit-order reversed.
 */

unsigned long dl_assemble_msg(dl_aircon_msg_t* msg){
  unsigned long buf = 0x12000000;

  buf |= bit_reverse(msg->timer_value) << 8;

  if (msg->on) buf |= 0x1 << 8;
  if (msg->timer) buf |= 0x1 << 9;

  if (msg->speed == 4 || msg->speed == 2 || msg->speed == 1){
    buf |= msg->speed << 20;
  }else{
    buf |= 0x2 << 20;
  }

  // TODO: Implement wind mode

  return buf;
}

bool dl_decode_msg(unsigned long code, dl_aircon_msg_t* msg){

  msg->on = ( (code) >> 8 ) & 0x01;
  msg->timer = ( (code) >> 9 ) & 0x01;

  msg->timer_value = bit_reverse((code >> 12) & 0xF) >> 4;
  msg->speed = (code >> 20) & 0xF;
}

void dl_print_msg(dl_aircon_msg_t *msg){
  DL_SERIAL.println("Settings:");

  if (msg->on){
    DL_SERIAL.print("ON  ");
  }else{
    DL_SERIAL.print("OFF ");
  }

  switch (msg->speed){
    case 1:
      DL_SERIAL.println("High ");
      break;
    case 2:
      DL_SERIAL.println("Mid  ");
      break;
    case 4:
      DL_SERIAL.println("Low  ");
      break;
  }

  switch (msg->wind){
    case 1:
      DL_SERIAL.println("Natural ");
      break;
    case 2:
      DL_SERIAL.println("Sleeping  ");
      break;
  }

  if (msg->timer){
    DL_SERIAL.print("Timer On:  ");
  }else{
    DL_SERIAL.print("Timer Off: ");
  }

  DL_SERIAL.println(msg->timer_value);
}

// Reverse the order of bits in a byte.
// I.e. MSB is swapped with LSB, etc.
unsigned char bit_reverse( unsigned char x )
{
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
   return x;
}
