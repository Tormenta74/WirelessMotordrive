/*
 * TimeSerialDateStrings.ino
 * example code illustrating Time library date strings
 *
 * This sketch adds date string functionality to TimeSerial sketch
 * Also shows how to handle different messages
 *
 * A message starting with a time header sets the time
 * A Processing example sketch to automatically send the messages is inclided in the download
 * On Linux, you can use "date +T%s\n > /dev/ttyACM0" (UTC time zone)
 *
 * A message starting with a format header sets the date format

 * send: Fs\n for short date format
 * send: Fl\n for long date format
 *
 * Modified and adapted to fit the LCD05 library by Diego Sáinz de Medrano.
 */

#include <lcd05.h>
#include <Wire.h>
#include <TimeLib.h>

#define LCD05_I2C_ADDRESS     byte((0xC6)>>1)

const int dt_SHORT_STR_LEN = 3;

// single character message tags
#define TIME_HEADER   'T'   // Header tag for serial time sync message
#define FORMAT_HEADER 'F'   // Header tag indicating a date format message
#define FORMAT_SHORT  's'   // short month and day strings
#define FORMAT_LONG   'l'   // (lower case l) long month and day strings

#define TIME_REQUEST  7     // ASCII bell character requests a time sync message

static boolean isLongFormat = true;

void fifo_wait() {
  while(lcd05::read_fifo_length(LCD05_I2C_ADDRESS)<4);
}

void lcdPrintDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  char digitStr[3];
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,":",1);
  if(digits < 10)
    sprintf(digitStr,"%02d",digits);
  else
    sprintf(digitStr,"%d",digits);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,digitStr,2);
}

void lcdDigitalClockDisplay() {
  // digital clock display of the time

  char messagerino[17];
  char digitStr[3];

  lcd05::clear_screen(LCD05_I2C_ADDRESS);

  sprintf(messagerino,"%d",hour());
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,2);

  lcdPrintDigits(minute());
  lcdPrintDigits(second());
  lcd05::ascii_chars(LCD05_I2C_ADDRESS," ",1);

  if(isLongFormat) {
    sprintf(messagerino,"%s",dayStr(weekday()));
    lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,
      strlen(dayStr(weekday())));
  } else {
    sprintf(messagerino,"%s",dayShortStr(weekday()));
    lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,dt_SHORT_STR_LEN);
  }

  lcd05::ascii_chars(LCD05_I2C_ADDRESS," ",1);
  if(day() < 10)
    sprintf(digitStr,"%02d",day());
  else
    sprintf(digitStr,"%d",day());

  lcd05::ascii_chars(LCD05_I2C_ADDRESS,digitStr,2);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS," ",1);

  if(isLongFormat) {
    sprintf(messagerino,"%s",monthStr(month()));
    lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,
      strlen(monthStr(month())));
  }
  else {
    sprintf(messagerino,"%s",monthShortStr(month()));
    lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,dt_SHORT_STR_LEN);
  }

  lcd05::ascii_chars(LCD05_I2C_ADDRESS," ",1);
  sprintf(messagerino,"%d",year());
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,messagerino,4);
}


void  processFormatMessage() {
  char c = Serial.read();
  if( c == FORMAT_LONG){
    isLongFormat = true;
    Serial.println(F("Setting long format"));
  }
  else if( c == FORMAT_SHORT) {
    isLongFormat = false;
    Serial.println(F("Setting short format"));
  }
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1498844769; //

  pctime = Serial.parseInt();
  if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
    setTime(pctime); // Sync Arduino clock to the time received on the serial port
  }
}

time_t requestSync() {
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}

void setup()  {

  Serial.begin(9600);
  setSyncProvider(requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");

  Wire.begin();
  delay(LCD05_I2C_INIT_DELAY);

  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::hide_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);
}

void loop(){
  if (Serial.available() > 1) { // wait for at least two characters
    char c = Serial.read();
    if( c == TIME_HEADER) {
      processSyncMessage();
    }
    else if( c== FORMAT_HEADER) {
      processFormatMessage();
    }
  }
  if (timeStatus()!= timeNotSet) {
    lcdDigitalClockDisplay();
  }
  delay(1000);
}
