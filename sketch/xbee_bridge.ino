/* xbee_bridge.ino
 * Author: Diego Sáinz de Medrano <diego.sainzdemedrano@gmail.com>
 *
 * This file is part of the Wireless Motordrive project.
 * Copyright (C) 2017 Diego Sáinz de Medrano.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation in its second version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, visit https://www.gnu.org/licenses/ to get
 * a copy.
 */

#define __STDC_LIMIT_MACROS

#include <avr/pgmspace.h>

#include <ctype.h>
#include <math.h>

#include <Wire.h>
#include <XBee.h>
#include <Time.h>
#include <lcd05.h>
#include <rd02.h>

#define SERIAL_BAUDS 38400

/*------------------------*/
/*-        Timer2        -*/
/*------------------------*/

#define TIMER1_TOP (int)(0.001 * (16e6 / 64.0)) - 1 // 249
#define TIMER1_OVF_TIME_MS 1
#define LCD_UPDATE_CYCLE 256   // 256ms
#define SPEED_CHECK_CYCLE 8    // 8ms

// timer variables
volatile uint64_t milliseconds = 0;
volatile bool lcd_update_flag = false;
volatile bool speed_check_flag = false;

// Timer2 interrupt vector; called every 1ms
ISR(TIMER2_OVF_vect)
{
  milliseconds++;
  if(milliseconds % LCD_UPDATE_CYCLE == 0) {
    lcd_update_flag = true;
  }
  if(milliseconds % SPEED_CHECK_CYCLE == 0) {
    speed_check_flag = true;
  }
}

// wrapped configuration registers access
void timer2_config()
{
  cli();

  // Timer2 settings:
  //   - CTC mode
  //   - 64 prescaler
  TCCR2A = 0x0;
  TCCR2A |= (1<<(WGM21));
  TCCR2A &= ~(~(1<<(WGM20)) & ~(1<<(WGM22)));
  TCCR2A |= (1<<(COM2A1)) | (1<<(COM2B1));

  TCCR2B &= ~(~(1<<(CS20)) & ~(1<<(CS21)));
  TCCR2B |= (1<<(CS22));

  OCR2A = TIMER1_TOP;
  TCNT2 = 0;
  TIMSK2 |= (1<<(TOIE2));

  sei();
}


/*----------------------*/
/*-        Xbee        -*/
/*----------------------*/
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
ZBTxRequest tx=ZBTxRequest();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

#define MAX_ZB_PAYLOAD_LENGTH 72 // according to xbee-arduino documentation

/*---------------------*/
/*-       LCD05       -*/
/*---------------------*/
#define LCD05_I2C_ADDRESS byte((0xC6)>>1)
#define LCD_LINE_LENGTH       16

// extra memory positions in the charset
#define LUP_ARROW_CHAR_POS    128
#define UP_ARROW_CHAR_POS     129
#define RUP_ARROW_CHAR_POS    130
#define RDOWN_ARROW_CHAR_POS  131
#define DOWN_ARROW_CHAR_POS   132
#define LDOWN_ARROW_CHAR_POS  133

// accesors for the char holder
#define LEFT_UP               0
#define UP                    1
#define RIGTH_UP              2
#define RIGHT                 3
#define RIGHT_DOWN            4
#define DOWN                  5
#define LEFT_DOWN             6
#define LEFT                  7
#define STOP                  8

// number of extra written chars
#define SPECIAL_CHARS_NUM     6

// bitmaps
#define CHAR_LINES       8
#define CHAR_LINE_LENGTH 8

const char ULA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10011110, 0b10011000, 0b10010100, 0b10010010, 0b10000001, 0b10000000, 0b10000000
};

const char UA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10000100, 0b10001110, 0b10010101, 0b10000100, 0b10000100, 0b10000100, 0b10000000
};


const char URA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10001111, 0b10000011, 0b10000101, 0b10001001, 0b10010000, 0b10000000, 0b10000000
};

const char DRA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10000000, 0b10010000, 0b10001001, 0b10000101, 0b10000011, 0b10001111, 0b10000000
};

const char DA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10000100, 0b10000100, 0b10000100, 0b10010101, 0b10001110, 0b10000100, 0b10000000
};

const char DLA_BYTES[CHAR_LINES] = {
  0b10000000, 0b10000000, 0b10000001, 0b10010010, 0b10010100, 0b10011000, 0b10011110, 0b10000000
};

const char direction_chars[SPECIAL_CHARS_NUM+3] = {
  LUP_ARROW_CHAR_POS, UP_ARROW_CHAR_POS, RUP_ARROW_CHAR_POS, 0b01111110, RDOWN_ARROW_CHAR_POS, DOWN_ARROW_CHAR_POS, LDOWN_ARROW_CHAR_POS, 0b01111111, 0b11011011
};

// drains the r/w register of the lcd
void fifo_wait()
{
  while(lcd05::read_fifo_length(LCD05_I2C_ADDRESS)<4);
}

// adds up, down and diagonal arrows
void add_arrow_chars()
{
  // reserving 8x8 bit matrix
  char *bytes_array[CHAR_LINES];
  for(int i=0; i<CHAR_LINES; i++)
    bytes_array[i] = (char*)malloc(CHAR_LINE_LENGTH*sizeof(char));

  // up left arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],ULA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,LUP_ARROW_CHAR_POS);

  // up arrow
  fifo_wait();
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],UA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,UP_ARROW_CHAR_POS);

  // up rigth arrow
  fifo_wait();
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],URA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,RUP_ARROW_CHAR_POS);

  // down rigth arrow
  fifo_wait();
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DRA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,RDOWN_ARROW_CHAR_POS);

  // down arrow
  fifo_wait();
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,DOWN_ARROW_CHAR_POS);

  // down left arrow
  fifo_wait();
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DLA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,LDOWN_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  for(int i=0; i<8; i++)
    if(bytes_array[i]) free(bytes_array[i]);
}

// populates the LCD starting from home
void top_left_corner(char *msg, int length)
{
  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

// populates the LCD top-right from "right to left"; that is,
// it will only write the last lenght positions of the corner
void top_right_corner(char *msg, int length)
{
  fifo_wait();

  lcd05::set_cursor(LCD05_I2C_ADDRESS, LCD_LINE_LENGTH - length + 1);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

// populates the LCD starting from just under home
void bottom_left_corner(char *msg, int length)
{
  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::cursor_vertical_tab(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

// populates the LCD bottom-right from "right to left"; that is,
// it will only write the last lenght positions of the corner
void bottom_right_corner(char *msg, int length)
{
  fifo_wait();

  lcd05::set_cursor(LCD05_I2C_ADDRESS, 2*LCD_LINE_LENGTH - length + 1);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

/*------------------------*/
/*-         RD02         -*/
/*------------------------*/
#define RD02_I2C_ADDRESS      byte((0xB0)>>1)
#define DUMMY_STRAIGHT_SPEED  32
#define DUMMY_TURN_SPEED      24
#define MAX_SPEED             127
#define SPEED_CONTROL_STEP    2

// buffers' positions
#define PREV                  0
#define CURR                  1

// drive modes
#define DUMMY                 0
#define NORMAL                1

// according to the RD02 documentation
#define WHEEL_DIAMETRE_MM     100

// this was a failed attempt: it works, but it is useless
// in calculations or printing, for that matter
//const int64_t CORRIDOR_LENGTH = INT32_MAX - INT32_MIN;

// drive mode controller
int drive_mode = DUMMY;
// commanded speed is in m/s; it was converted when we parsed the command
double commanded_speed = 0.0, measured_speed1=0.0, measured_speed2=0.0;

// this is what will be written to the corresponding registers
// these vars are manipulated in the regulate_speed function
int8_t speed_code1=0, speed_code2=0;
// storage for the motor encoding readings
int32_t enc1[2]={0}, enc2[2]={0};
// storage for the timestamps (in ms)
uint64_t time1[2]={0}, time2[2]={0};

// each tick on the encoder means one degree (in 360º) of advance
// on the wheel. given that we know the diametre of the wheel, we
// can calculate how much distance is traveled per tick
const double ADVANCE_PER_TICK_MM = (double)WHEEL_DIAMETRE_MM*M_PI/(double)360;

// proportional speed control constant (see regulate_speed)
double K = 10.00;

// int64_t implementation of abs()
int64_t i64abs(int64_t num)
{
  if(num < 0)
    return -num;
  return num;
}

// measures the current speed of the motor by reading the
// encoder and registering the times, comparing to the desired
// speed and augmenting/decrementing the speed codes
void regulate_speed()
{
  int64_t delta1=0, delta2=0, delta1c=0, delta2c=0;
  double inc=0.0;

  // register time
  time1[CURR] = milliseconds;

  // get new encoder value
  enc1[CURR] = rd02::read_enc1(RD02_I2C_ADDRESS);

  // compare with previous
  delta1 = enc1[CURR] - enc1[PREV];
  delta1c = INT32_MAX - i64abs(delta1) - INT32_MIN;
  // in case of overflow
  if(i64abs(delta1) > i64abs(delta1c)) {
    if(delta1 < 0)
      delta1 = delta1c;
    else
      delta1 = -delta1c;
  } // else case is default

  // get corresponding velocity (in mm/ms)
  measured_speed1 = static_cast<double>(delta1) * ADVANCE_PER_TICK_MM / (time1[CURR] - time1[PREV]);

  // compare with target 
  inc = K*(commanded_speed-measured_speed1);
  if((double)speed_code1 + inc > MAX_SPEED) {
    speed_code1 = MAX_SPEED;
  } else if((double)speed_code1 + inc < -MAX_SPEED) {
    speed_code1 = -MAX_SPEED;
  } else {
    speed_code1 += inc;
  }

  // debug information
  if(lcd_update_flag) {
    Serial.print("\ntarget speed (m/s) = "); Serial.println(commanded_speed,6);
    Serial.print("speed1 (m/s) = "); Serial.println(measured_speed1,6);
    Serial.print("increment = "); Serial.println(K*(commanded_speed-measured_speed1));
    Serial.print("new speed1 code = "); Serial.println((int)speed_code1);
  }


  // --------------------
  // repeat for encoder 2
  time2[CURR] = milliseconds;
  enc2[CURR] = rd02::read_enc2(RD02_I2C_ADDRESS);
  delta2 = enc2[CURR] - enc2[PREV];
  delta2c = INT32_MAX - i64abs(delta2) - INT32_MIN;
  if(i64abs(delta2) > i64abs(delta2c)) {
    if(delta2 < 0)
      delta2 = delta2c;
    else
      delta2 = -delta2c;
  }
  measured_speed2 = static_cast<double>(delta2) * ADVANCE_PER_TICK_MM / (time2[CURR] - time2[PREV]);
  inc = K*(commanded_speed-measured_speed2);
  if((double)speed_code2 + inc > MAX_SPEED) {
    speed_code2 = MAX_SPEED;
  } else if((double)speed_code2 + inc < -MAX_SPEED) {
    speed_code2 = -MAX_SPEED;
  } else {
    speed_code2 += inc;
  }
  if(lcd_update_flag) {
    Serial.print("\nspeed2 (m/s) = "); Serial.println(measured_speed2,6);
    Serial.print("new speed2 code = "); Serial.println((int)speed_code2);
  }

  // store encoder values for future measurement
  enc1[PREV] = enc1[CURR]; time1[PREV] = time1[CURR];
  enc2[PREV] = enc2[CURR]; time2[PREV] = time2[CURR];
}

// simple control center to always manage the RD02 motors
void motor_control()
{
  // regulate speed codes
  if(drive_mode == NORMAL) {
    regulate_speed();
  }
  // always set speeds
  rd02::set_speed1(RD02_I2C_ADDRESS,speed_code1);
  rd02::set_speed2(RD02_I2C_ADDRESS,speed_code2);
  // reset flag
  speed_check_flag = false;
}


/*----------------------*/
/*-        Time        -*/
/*----------------------*/
#define DATE_MSG_LENGTH       5

// set time
void processSyncMessage(time_t pctime)
{
  setTime(pctime);
  Serial.print("Time set! "); Serial.println(pctime,DEC);
}

/*------------------------*/
/*-        SKETCH        -*/
/*------------------------*/

void setup()
{
  // start services
  Serial.begin(SERIAL_BAUDS);
  xbee.begin(Serial);
  Wire.begin();
  timer2_config();

  // hardware delays
  delay(LCD05_I2C_INIT_DELAY);

  // hardware configs

  Serial.print(F("setting up LCD05 with address 0x"));
  Serial.print(LCD05_I2C_ADDRESS<<1,HEX);
  Serial.println(F("..."));
  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::hide_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);
  Serial.println(F("done."));

  Serial.print(F("adding special characters..."));
  add_arrow_chars();
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  Serial.println(F("done."));

  Serial.print(F("configuring RD02 with address 0x"));
  Serial.print(RD02_I2C_ADDRESS<<1,HEX);
  Serial.println(F("..."));
  rd02::set_mode(RD02_I2C_ADDRESS, 1);
  rd02::set_speed1(RD02_I2C_ADDRESS, 0);
  rd02::set_speed2(RD02_I2C_ADDRESS, 0);
  rd02::reset_encoders(RD02_I2C_ADDRESS);
  // warning: this calls are blocking, and you will not be
  // able to run this program if the RD02 is not wired
  time1[PREV] = milliseconds;
  enc1[PREV] = rd02::read_enc1(RD02_I2C_ADDRESS);
  time2[PREV] = milliseconds;
  enc2[PREV] = rd02::read_enc2(RD02_I2C_ADDRESS);
  Serial.println(F("done."));

  // start function: request time
  Serial.print(F("configuring time (requesting UTC time)..."));

  // prompt
  top_left_corner((char*)" start console",14);
}

// contains the to right corner displayed character
char direction_buffer[1] = {direction_chars[STOP]};

// continuously reads packets, looking for ZB Receive or Modem Status
void loop()
{
  char lcd_buffer[LCD_LINE_LENGTH];
  uint8_t payload[MAX_ZB_PAYLOAD_LENGTH];

  // do not run anything until the console has connected and time has been set
  if(!(timeStatus() != timeSet)) {
    // each SPEED_CHECK_CYCLE ms
    if(speed_check_flag) {
      motor_control();
    }

    // each LCD_UPDATE_CYCLE ms
    // update liquid crystal display
    if(lcd_update_flag) {
      lcd05::clear_screen(LCD05_I2C_ADDRESS);

      // top left
      if(drive_mode == DUMMY) {
          sprintf(lcd_buffer,"S1:%d",speed_code1);
      } else {
          sprintf(lcd_buffer,"M1:% 2.1fm/s",measured_speed1);
      }
      top_left_corner(lcd_buffer,strlen(lcd_buffer));

      // bottom left
      if(drive_mode == DUMMY) {
          sprintf(lcd_buffer,"S2:%d",speed_code2);
      } else {
          sprintf(lcd_buffer,"M2:% 2.1fm/s",measured_speed2);
      }
      bottom_left_corner(lcd_buffer,strlen(lcd_buffer));

      // top right
      top_right_corner(direction_buffer,1);

      // bottom right
      sprintf(lcd_buffer,"%02d:%02d",hour(),minute());
      bottom_right_corner(lcd_buffer,DATE_MSG_LENGTH);

      // reset the update flag
      lcd_update_flag = false;
    }
  }

  // just in case (without this, payload used to cause problems)
  memset(payload,0,MAX_ZB_PAYLOAD_LENGTH*sizeof(int8_t));
  // listen to the network
  xbee.readPacket();

  if(xbee.getResponse().isAvailable()) { // got something

    switch(xbee.getResponse().getApiId()) {
    case ZB_RX_RESPONSE:

      xbee.getResponse().getZBRxResponse(rx);

      // get data in manageable format
      memcpy(
          payload,
          rx.getData(),
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );

      // sending the echo back
      // echo is equivalent to acknowledged in console application
      tx=ZBTxRequest(
          rx.getRemoteAddress64(),
          payload,
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );
      xbee.send(tx);

      // process command prefix

      // dummy mode: process arrow commands
      if(*(char*)payload == 'D') {
        drive_mode = DUMMY;
        commanded_speed = 0.0;
        switch(*((char*)payload+1)) {
        case '0':
          speed_code1 = speed_code2 = 0;
          direction_buffer[0] = direction_chars[STOP];
          break;
        case '1':
          speed_code1 = speed_code2 = DUMMY_STRAIGHT_SPEED;
          direction_buffer[0] = direction_chars[UP];
          break;
        case '2':
          speed_code1 = speed_code2 = -DUMMY_STRAIGHT_SPEED;
          direction_buffer[0] = direction_chars[DOWN];
          break;
        case '3':
          speed_code1 = DUMMY_TURN_SPEED;
          speed_code2 = -DUMMY_TURN_SPEED;
          direction_buffer[0] = direction_chars[LEFT];
          break;
        case '4':
          speed_code1 = -DUMMY_TURN_SPEED;
          speed_code2 = DUMMY_TURN_SPEED;
          direction_buffer[0] = direction_chars[RIGHT];
          break;
        default:
          direction_buffer[0] = direction_chars[STOP];
        }
      } else if(*(char*)payload == 'S') { // process speed message
        // commanded speed comes in in mm / s
        drive_mode = NORMAL;
        direction_buffer[0] = 'N';
        commanded_speed = (double)atoi((char*)(payload+1))/1000.0;

        Serial.print("setting speed to "); Serial.print(commanded_speed);
        Serial.println("m/s");
      } else if(*(char*)payload == 'T') { // process time sync message
        time_t time = atol((char*)payload+1);
        processSyncMessage(time);
      } else if(*(char*)payload == 'K') { // process K set message
        int k = atoi((char*)payload+1);
        K = 1.0*k;
      }

      // rx log

      Serial.print("\n----> remote address: 0x");
      Serial.print(rx.getRemoteAddress64().getMsb(),HEX);
      Serial.print(",0x");
      Serial.println(rx.getRemoteAddress64().getLsb(),HEX);
      Serial.print("----> lenght: ");
      Serial.println(rx.getDataLength());
      Serial.print("----> acknowledged: ");
      Serial.println(
          (rx.getOption()==ZB_PACKET_ACKNOWLEDGED)? "yes": "no"
          );
      Serial.print("----> data: ");
      Serial.println((char*)payload);

      break;

      // xbee special cases
    case MODEM_STATUS_RESPONSE:
      xbee.getResponse().getModemStatusResponse(msr);
      // the local XBee sends this response on certain events, like association/dissociation
      break;
    case ZB_TX_STATUS_RESPONSE:
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      break;
    default:;
    }
  }
  else {
    if (xbee.getResponse().isError()) {
    }
  }
}
