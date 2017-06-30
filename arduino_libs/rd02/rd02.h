
#ifndef rd03_h
#define rd03_h

#include <Arduino.h>

#define SPEED1_REGISTER           byte(0) // mode 0,1
#define SPEED2_REGISTER           byte(1) // mode 0,1
#define SPEED_REGISTER            byte(0) // mode 2,3
#define TURN_REGISTER             byte(1) // mode 2,3

#define ENC1A_REGISTER            byte(2)
#define ENC1B_REGISTER            byte(3)
#define ENC1C_REGISTER            byte(4)
#define ENC1D_REGISTER            byte(5)

#define ENC2A_REGISTER            byte(6)
#define ENC2B_REGISTER            byte(7)
#define ENC2C_REGISTER            byte(8)
#define ENC2D_REGISTER            byte(9)

#define BATTERY_VOLTS_REGISTER    byte(10)
#define MOTOR1_CURRENT_REGISTER   byte(11)
#define MOTOR2_CURRENT_REGISTER   byte(12)
#define VERSION_REGISTER          byte(13)

#define ACCELERATION_REGISTER     byte(14)
#define MODE_REGISTER             byte(15)

#define RD02_COMMAND_REGISTER     byte(16)

#define RESET_ENCODERS            byte(32)
#define DISABLE_SPEED_REGULATION  byte(48)
#define ENABLE_SPEED_REGULATION   byte(49)

#define ADDR_CHANGE_SEQ_1         byte(0xA0)
#define ADDR_CHANGE_SEQ_2         byte(0xAA)
#define ADDR_CHANGE_SEQ_3         byte(0xA5)

namespace rd02 {
  int8_t get_speed1(byte address);
  int8_t get_speed2(byte address);
  int8_t get_speed(byte address);
  int8_t get_turn(byte address);
  void set_speed1(byte address, int8_t speed);
  void set_speed2(byte address, int8_t speed);
  void set_speed(byte address, int8_t speed);
  void set_turn(byte address, int8_t turn);
  int32_t read_enc1(byte address);
  int32_t read_enc2(byte address);
  int read_battery(byte address);
  int read_current1(byte address);
  int read_current2(byte address);
  int get_software_rev(byte address);
  int get_acceleration(byte address);
  void set_acceleration(byte address, int acc);
  void set_mode(byte address, int mode);
  void reset_encoders(byte address);
  void disable_speed_regulation(byte address);
  void enable_speed_regulation(byte address);
  //void disable_i2c_timeout(byte address);
  //void enable_i2c_timeout(byte address);
}

#endif
