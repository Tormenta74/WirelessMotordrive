/* lcd05.cpp
 * Example showing how to use the Devantech LCD05 LCD 16x2 display
 * in I2C mode. More info about this LCD display in
 *   http://www.robot-electronics.co.uk/htm/Lcd05tech.htm
 * author: Antonio C. Domínguez Brito <adominguez@iusiani.ulpgc.es>
 */

/* Arduino I2C pins
 * Analog 4 -> SDA (I2C data line)
 * Analgo 5 -> SCL (I2C clock line)
 */

#include <Arduino.h>
#include <Wire.h>   // this forces the sketch to include Wire.h
#include "lcd05.h"

inline void write_command(byte command)
{ Wire.write(LCD05_COMMAND_REGISTER); Wire.write(command); }

void lcd05::set_display_type(byte address, byte type)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(SET_DISPLAY_TYPE);
  Wire.write(type);
  Wire.endTransmission();
}

void lcd05::set_tab_length(byte address, byte length)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(TAB_SET);
  Wire.write(length);
  Wire.endTransmission();
}

void lcd05::clear_screen(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(CLEAR_SCREEN);
  Wire.endTransmission();
}

void lcd05::cursor_home(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(CURSOR_HOME);
  Wire.endTransmission();
}

void lcd05::cursor_vertical_tab(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(VERTICAL_TAB);
  Wire.endTransmission();
}

void lcd05::cursor_horizontal_tab(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(HORIZONTAL_TAB);
  Wire.endTransmission();
}

void lcd05::set_cursor(byte address, byte pos)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(CURSOR_HOME);
  Wire.write(pos);
  Wire.endTransmission();
}

void lcd05::set_cursor_coords(byte address, byte line, byte column)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(CURSOR_HOME);
  Wire.write(line);
  Wire.write(column);
  Wire.endTransmission();
}

void lcd05::show_blinking_cursor(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(SHOW_BLINKING_CURSOR);
  Wire.endTransmission();
}

void lcd05::hide_cursor(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(HIDE_CURSOR);
  Wire.endTransmission();
}

void lcd05::backlight_on(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(BACKLIGHT_ON);
  Wire.endTransmission();
}

void lcd05::backlight_off(byte address)
{
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(BACKLIGHT_OFF);
  Wire.endTransmission();
}

bool lcd05::add_special_char(byte address, char **bytes, byte pos)
{
  if(pos<128 || pos>135) return false;
  Wire.beginTransmission(address); // start communication with LCD 05
  write_command(CUSTOM_CHAR_GENERATOR);
  Wire.write(pos);
  for(int i=0; i<8; i++)
    for(int j=0; j<8; j++)
      Wire.write(bytes[i][j]);
  Wire.endTransmission();
  return true;
}

bool lcd05::ascii_chars(byte address, char* bytes, int length)
{
  if(length<=0) return false;
  Wire.beginTransmission(address); // start communication with LCD 05
  Wire.write(COMMAND_REGISTER);
  for(int i=0; i<length; i++, bytes++) Wire.write(*bytes);
  Wire.endTransmission();
  return true;
}

byte lcd05::read_fifo_length(byte address)
{
  Wire.beginTransmission(address);
  Wire.write(FIFO_AVAILABLE_LENGTH_REGISTER);                           // Call the register for start of ranging data
  Wire.endTransmission();

  Wire.requestFrom(address,byte(1)); // start communication with LCD 05, request one byte
  while(!Wire.available()) { /* do nothing */ }
  return Wire.read();
}
