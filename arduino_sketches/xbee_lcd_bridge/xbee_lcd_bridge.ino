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

#include <ctype.h>

#include <Wire.h>
#include <XBee.h>
#include <Time.h>
#include <lcd05.h>
#include <rd02.h>

#define SERIAL_BAUDS 38400

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

char holder[SPECIAL_CHARS_NUM+2] = {
  LUP_ARROW_CHAR_POS, UP_ARROW_CHAR_POS, RUP_ARROW_CHAR_POS, 0b01111110, RDOWN_ARROW_CHAR_POS, DOWN_ARROW_CHAR_POS, LDOWN_ARROW_CHAR_POS, 0b01111111
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
#define RD02_I2C_ADDRESS  byte((0xB0)>>1)

int drive_mode = 0;

/*------------------------*/
/*-        Timer2        -*/
/*------------------------*/
#define TIMER1_TOP (int)(0.001 * (16e6 / 64.0)) - 1 // 249
#define TIMER1_OVF_TIME_MS 1
#define MILLISECONDS_IN_SECOND 1000

// timer variables
volatile uint64_t milliseconds = 0;
volatile bool update_flag = false;

ISR(TIMER2_OVF_vect) {
  milliseconds++;
  if(milliseconds % MILLISECONDS_IN_SECOND == 0) {
    update_flag = true;
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

  TCCR2B &= ~(~(1<<(CS20)) & ~(1<<(CS21)));
  TCCR2B |= (1<<(CS22));

  OCR2A = TIMER1_TOP;
  TCNT2 = 0;
  TIMSK2 |= (1<<(TOIE2));

  sei();
}

/*----------------------*/
/*-        Time        -*/
/*----------------------*/
#define START_YEAR            1970

const int dt_SHORT_STR_LEN = 3;

// single character message tags
#define TIME_HEADER   'T'   // Header tag for serial time sync message
#define FORMAT_HEADER 'F'   // Header tag indicating a date format message
#define FORMAT_SHORT  's'   // short month and day strings
#define FORMAT_LONG   'l'   // (lower case l) long month and day strings

#define TIME_REQUEST  7     // ASCII bell character requests a time sync message

static boolean isLongFormat = true;
const unsigned long DEFAULT_TIME = 1498932320; // sat jul 1 18:05:45 UTC 2017

void processFormatMessage() {
  char c = Serial.read();
  if(c == FORMAT_LONG){
    isLongFormat = true;
    Serial.println(F("Setting long format"));
  }
  else if(c == FORMAT_SHORT) {
    isLongFormat = false;
    Serial.println(F("Setting short format"));
  }
}

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
  Serial.print("setting up LCD05 with address 0x");
  Serial.print(LCD05_I2C_ADDRESS<<1,HEX);
  Serial.println("...");
  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::hide_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);
  Serial.println("done.");

  Serial.print("adding special characters...");
  add_arrow_chars();
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  Serial.println("done.");

  Serial.print("configuring RD02 with address 0x");
  Serial.print(RD02_I2C_ADDRESS<<1,HEX);
  Serial.println("...");
  rd02::reset_encoders(RD02_I2C_ADDRESS);
  //rd02::disable_speed_regulation(RD02_I2C_ADDRESS);
  rd02::set_mode(RD02_I2C_ADDRESS, 1);
  rd02::set_speed1(RD02_I2C_ADDRESS, 0);
  rd02::set_speed2(RD02_I2C_ADDRESS, 0);
  Serial.println("done.");

  // start function: request time
  Serial.print("configuring time (requesting UTC time)...");
  setSyncProvider(requestSync);  //set function to call when sync required
}

// continuously reads packets, looking for ZB Receive or Modem Status
void loop() 
{    
  int i;

  switch(drive_mode) {
  case 1:
    rd02::set_speed1(RD02_I2C_ADDRESS,32);
    rd02::set_speed2(RD02_I2C_ADDRESS,32);
    break;
  case 2:
    rd02::set_speed1(RD02_I2C_ADDRESS,-32);
    rd02::set_speed2(RD02_I2C_ADDRESS,-32);
    break;
  case 3:
    rd02::set_speed1(RD02_I2C_ADDRESS,16);
    rd02::set_speed2(RD02_I2C_ADDRESS,-16);
    break;
  case 4:
    rd02::set_speed1(RD02_I2C_ADDRESS,-16);
    rd02::set_speed2(RD02_I2C_ADDRESS,16);
    break;
  default:
    rd02::set_speed1(RD02_I2C_ADDRESS,0);
    rd02::set_speed2(RD02_I2C_ADDRESS,0);
  }

  xbee.readPacket();

  if (xbee.getResponse().isAvailable()) 
  { // got something

    switch(xbee.getResponse().getApiId())
    {
    case ZB_RX_RESPONSE:

      xbee.getResponse().getZBRxResponse(rx);
      Serial.println("--> data received:");
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

      memcpy(
          payload,
          rx.getData(),
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );

      Serial.print("----> data: \"");
      for(i=0; i<min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength()); i++) 
        Serial.print(char(payload[i]));
      Serial.println("\"");

      // sending the echo back
      tx=ZBTxRequest(
          rx.getRemoteAddress64(),
          payload,
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );
      xbee.send(tx);       

      lcd05::clear_screen(LCD05_I2C_ADDRESS);
      /*
         lcd05::ascii_chars(
         LCD05_I2C_ADDRESS,
         (char*)payload,
         min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
         );
       */

      char holder[1], tmp[1];
      tmp[0] = payload[0];
      if(isdigit(tmp[0])) {
        drive_mode = atoi(tmp);
        Serial.println();
        Serial.println(drive_mode);
        switch(tmp[0]) {
        case '1':
          top_right_corner(holder,1);
          break;
        case '2':
          holder[0] = DOWN_ARROW_CHAR_POS;
          top_right_corner(holder,1);
          break;
        case '3':
          holder[0] = LUP_ARROW_CHAR_POS;
          top_right_corner(holder,1);
          break;
        case '4':
          holder[0] = RDOWN_ARROW_CHAR_POS;
          top_right_corner(holder,1);
          break;
        default:
          drive_mode = 0;
        }
      }


      break;

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
