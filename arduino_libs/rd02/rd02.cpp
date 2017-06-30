
#include <Wire.h>
#include "rd02.h"

inline void write_command(byte command)
{ Wire.write(RD02_COMMAND_REGISTER); Wire.write(command); }

int8_t rd02::get_speed1(byte address) {
  Wire.beginTransmission(address);
  Wire.write(SPEED1_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1));
  while(!Wire.available());
  return Wire.read();
}

int8_t rd02::get_speed2(byte address) {
  Wire.beginTransmission(address);
  Wire.write(SPEED2_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1));
  while(!Wire.available());
  return Wire.read();
}

int8_t rd02::get_speed(byte address) {
  Wire.beginTransmission(address);
  Wire.write(SPEED_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1));
  while(!Wire.available());
  return Wire.read();
}

int8_t rd02::get_turn(byte address) {
  Wire.beginTransmission(address);
  Wire.write(TURN_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1));
  while(!Wire.available());
  return Wire.read();
}

void rd02::set_speed1(byte address, int8_t speed) {
  Wire.beginTransmission(address);
  Wire.write(SPEED1_REGISTER);
  Wire.write(speed);
  Wire.endTransmission();
}

void rd02::set_speed2(byte address, int8_t speed) {
  Wire.beginTransmission(address);
  Wire.write(SPEED2_REGISTER);
  Wire.write(speed);
  Wire.endTransmission();
}

void rd02::set_speed(byte address, int8_t speed) {
  Wire.beginTransmission(address);
  Wire.write(SPEED_REGISTER);
  Wire.write(speed);
  Wire.endTransmission();
}

void rd02::set_turn(byte address, int8_t turn) {
  Wire.beginTransmission(address);
  Wire.write(TURN_REGISTER);
  Wire.write(turn);
  Wire.endTransmission();
}

/* https://www.robot-electronics.co.uk/files/arduino_md25_i2c.ino */
int32_t rd02::read_enc1(byte address) {
  int32_t result = 0;

  Wire.beginTransmission(address);
  Wire.write(ENC1A_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(4));
  while(Wire.available() < 4);

  for(int i=0; i<4; i++) {
    result |= Wire.read();
    if(i==3)
      break;
    result <<= 8;
  }
  return result;
}

/* https://www.robot-electronics.co.uk/files/arduino_md25_i2c.ino */
int32_t rd02::read_enc2(byte address) {
  int32_t result = 0;

  Wire.beginTransmission(address);
  Wire.write(ENC2A_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(4));
  while(Wire.available() < 4);

  for(int i=0; i<4; i++) {
    result |= Wire.read();
    if(i==3)
      break;
    result <<= 8;
  }
  return result;
}

int rd02::get_software_rev(byte address) {
  Wire.beginTransmission(address);
  Wire.write(MODE_REGISTER);
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1));
  while(!Wire.available());
  return Wire.read();
}

void rd02::set_mode(byte address, int mode) {
  Wire.beginTransmission(address);
  Wire.write(MODE_REGISTER);
  Wire.write(mode);
  Wire.endTransmission();
}

void rd02::reset_encoders(byte address) {
  Wire.beginTransmission(address);
  write_command(RESET_ENCODERS);
  Wire.endTransmission();
}

void rd02::disable_speed_regulation(byte address) {
  Wire.beginTransmission(address);
  write_command(DISABLE_SPEED_REGULATION);
  Wire.endTransmission();
}

void rd02::enable_speed_regulation(byte address) {
  Wire.beginTransmission(address);
  write_command(ENABLE_SPEED_REGULATION);
  Wire.endTransmission();
}
