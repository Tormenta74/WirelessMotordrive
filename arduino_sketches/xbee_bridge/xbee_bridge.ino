/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/pgmspace.h>

#include <ctype.h>

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
#define SPEED_CHECK_CYCLE 4   // 4ms

// timer variables
volatile uint64_t milliseconds = 0;
volatile bool lcd_update_flag = false;
volatile bool speed_check_flag = false;

/*ISR(TIMER2_COMPA_vect) {
  milliseconds++;
  if(milliseconds & LCD_UPDATE_CYCLE == 0) {
  lcd_update_flag = true;
  }
  if(milliseconds & SPEED_CHECK_CYCLE == 0) {
  speed_check_flag = true;
  }
  }*/

ISR(TIMER2_OVF_vect) {
  milliseconds++;
  if(milliseconds % LCD_UPDATE_CYCLE == 0) {
    lcd_update_flag = true;
  }
  if(milliseconds % SPEED_CHECK_CYCLE == 0) {
    speed_check_flag = true;
  }
}

void timer2_config() {
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
uint8_t payload[MAX_ZB_PAYLOAD_LENGTH];

/*---------------------*/
/*-       LCD05       -*/
/*---------------------*/
#define LCD05_I2C_ADDRESS byte((0xC6)>>1)
#define MAX_LCD_MESSAGE       7
#define LCD_TAB_LENGTH        8
#define LCD_LINE_LENGTH       16

#define LUP_ARROW_CHAR_POS    128
#define UP_ARROW_CHAR_POS     129
#define RUP_ARROW_CHAR_POS    130
#define RDOWN_ARROW_CHAR_POS  131
#define DOWN_ARROW_CHAR_POS   132
#define LDOWN_ARROW_CHAR_POS  133

#define LEFT_UP               0
#define UP                    1
#define RIGTH_UP              2
#define RIGHT                 3
#define RIGHT_DOWN            4
#define DOWN                  5
#define LEFT_DOWN             6
#define LEFT                  7
#define STOP                  8

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

char direction_chars[SPECIAL_CHARS_NUM+3] = {
  LUP_ARROW_CHAR_POS, UP_ARROW_CHAR_POS, RUP_ARROW_CHAR_POS, 0b01111110, RDOWN_ARROW_CHAR_POS, DOWN_ARROW_CHAR_POS, LDOWN_ARROW_CHAR_POS, 0b01111111, 0b11011011
};

void fifo_wait() {
  while(lcd05::read_fifo_length(LCD05_I2C_ADDRESS)<4);
}

// adds up, down and diagonal arrows
void add_arrow_chars() {
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

void top_left_corner(char *msg, int length) {
  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void top_right_corner(char *msg, int length) {
  fifo_wait();

  lcd05::set_cursor(LCD05_I2C_ADDRESS, LCD_LINE_LENGTH - length + 1);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void bottom_left_corner(char *msg, int length) {
  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::cursor_vertical_tab(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void bottom_right_corner(char *msg, int length) {
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

int drive_mode = 0;

int32_t prev_enc1=0, prev_enc2=0;
int32_t enc1=0, enc2=0;

// this is real m/s times 100
int speed_ms = 0;

void regulate_speed() {
  // get new encoder values
  // compare with previous
  // divide by time (4ms)
}

void motor_control() {
  if(speed_check_flag) {
    switch(drive_mode) {
    case 0:
      rd02::set_speed1(RD02_I2C_ADDRESS,0);
      rd02::set_speed2(RD02_I2C_ADDRESS,0);
      break;
    case 1:
      rd02::set_speed1(RD02_I2C_ADDRESS,DUMMY_STRAIGHT_SPEED);
      rd02::set_speed2(RD02_I2C_ADDRESS,DUMMY_STRAIGHT_SPEED);
      break;
    case 2:
      rd02::set_speed1(RD02_I2C_ADDRESS,-DUMMY_STRAIGHT_SPEED);
      rd02::set_speed2(RD02_I2C_ADDRESS,-DUMMY_STRAIGHT_SPEED);
      break;
    case 3:
      rd02::set_speed1(RD02_I2C_ADDRESS,DUMMY_TURN_SPEED);
      rd02::set_speed2(RD02_I2C_ADDRESS,-DUMMY_TURN_SPEED);
      break;
    case 4:
      rd02::set_speed1(RD02_I2C_ADDRESS,-DUMMY_TURN_SPEED);
      rd02::set_speed2(RD02_I2C_ADDRESS,DUMMY_TURN_SPEED);
      break;
    default:
      regulate_speed();
    }
  }
}


/*----------------------*/
/*-        Time        -*/
/*----------------------*/
#define START_YEAR            1970
#define DATE_MSG_LENGTH       5

const int dt_SHORT_STR_LEN = 3;

// single character message tags
#define TIME_HEADER   'T'   // Header tag for serial time sync message
#define FORMAT_HEADER 'F'   // Header tag indicating a date format message
#define FORMAT_SHORT  's'   // short month and day strings
#define FORMAT_LONG   'l'   // (lower case l) long month and day strings

#define TIME_REQUEST  7     // ASCII bell character requests a time sync message

static boolean isLongFormat = true;
const unsigned long DEFAULT_TIME = 1498932320; // sat jul 1 18:05:45 UTC 2017

void processSyncMessage(unsigned long pctime) {
  if(pctime >= DEFAULT_TIME) {
    setTime(pctime); // Sync Arduino clock to the time received on the serial port
  }
}

time_t requestSync() {
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
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
  rd02::reset_encoders(RD02_I2C_ADDRESS);
  //rd02::disable_speed_regulation(RD02_I2C_ADDRESS);
  rd02::set_mode(RD02_I2C_ADDRESS, 1);
  rd02::set_speed1(RD02_I2C_ADDRESS, 0);
  rd02::set_speed2(RD02_I2C_ADDRESS, 0);
  Serial.println(F("done."));

  // start function: request time
  Serial.print(F("configuring time (requesting UTC time)..."));
  setSyncProvider(requestSync);  //set function to call when sync required
  //setTime(DEFAULT_TIME);
  top_left_corner((char*)" start console",14);
}

char direction_buffer[1];

// continuously reads packets, looking for ZB Receive or Modem Status
void loop()
{
  char date_buffer[DATE_MSG_LENGTH];

  if(timeStatus() == timeSet) {
    motor_control();

    // update liquid crystal display
    if(lcd_update_flag) {
      lcd05::clear_screen(LCD05_I2C_ADDRESS);

      // ...
      if(drive_mode <= 4)
        top_right_corner(direction_buffer,1);

      sprintf(date_buffer,"%02d:%02d",hour(),minute());
      bottom_right_corner(date_buffer,DATE_MSG_LENGTH);
      lcd_update_flag = false;
    }
  }


  // listen to the network
  xbee.readPacket();

  if(xbee.getResponse().isAvailable()) 
  { // got something

    switch(xbee.getResponse().getApiId())
    {
    case ZB_RX_RESPONSE:

      xbee.getResponse().getZBRxResponse(rx);

      // get data in manageable format
      memcpy(
          payload,
          rx.getData(),
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );

      // verify it is epoch
      if(rx.getDataLength() > 1 && isdigit(*((char*)payload))) { // dereferencing the first mem address
        processSyncMessage(atol((char*)payload));
      }

      // sending the echo back
      // echo is equivalent to acknowledged in console application
      tx=ZBTxRequest(
          rx.getRemoteAddress64(),
          payload,
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );
      xbee.send(tx);       

      // switch mode
      // normal mode: process possible commands

      // dummy mode: process arrow commands
      if(isdigit(*((char*)payload))) {
        switch(*((char*)payload)) {
        case '1':
          drive_mode = 1;
          direction_buffer[0] = direction_chars[UP];
          break;
        case '2':
          drive_mode = 2;
          direction_buffer[0] = direction_chars[DOWN];
          break;
        case '3':
          drive_mode = 3;
          direction_buffer[0] = direction_chars[LEFT];
          break;
        case '4':
          drive_mode = 4;
          direction_buffer[0] = direction_chars[RIGHT];
          break;
        default:
          direction_buffer[0] = direction_chars[STOP];
          drive_mode = 0;
        }
      }


      // rx log
      Serial.print("----> remote address: 0x");
      Serial.print(rx.getRemoteAddress64().getMsb(),HEX);
      Serial.print(",0x");
      Serial.println(rx.getRemoteAddress64().getLsb(),HEX);
      Serial.print("----> lenght: ");
      Serial.println(rx.getDataLength());
      Serial.print("----> acknowledged: ");
      Serial.println(
          (rx.getOption()==ZB_PACKET_ACKNOWLEDGED)? "yes": "no"
          );


      break;

      // xbee special cases
    case MODEM_STATUS_RESPONSE:
      xbee.getResponse().getModemStatusResponse(msr);
      // the local XBee sends this response on certain events, like association/dissociation

      Serial.print("--> modem status received (0x");
      Serial.print(msr.getStatus(),HEX);
      Serial.println("):");
      if(msr.getStatus()==ASSOCIATED) Serial.println("----> associated");
      else if (msr.getStatus()==DISASSOCIATED) Serial.println("----> disassociated");
      else Serial.println("----> other status ");
      break;

    case ZB_TX_STATUS_RESPONSE:
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      Serial.print("--> tx status received (0x");
      Serial.print(txStatus.getDeliveryStatus(),HEX);
      Serial.print("): ");
      Serial.println(
          (txStatus.getDeliveryStatus()==SUCCESS)? "delivered": "not delivered"
          );
      break;

    default:
      Serial.print("--> something received (0x");
      Serial.print(xbee.getResponse().getApiId(),HEX);
      Serial.println(")");
    }
  } 
  else
  { 
    if (xbee.getResponse().isError()) 
    {
      Serial.print("--> error reading packet, errror code: 0x");  
      Serial.print(xbee.getResponse().getErrorCode(),HEX);
      Serial.println(")");
    }
  }
}
