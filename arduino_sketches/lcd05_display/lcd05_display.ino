#include <lcd05.h>
#include <Wire.h>

#define SERIAL_BAUDS        9600

#define LCD05_I2C_ADDRESS   byte((0xC6)>>1)
#define MAX_LCD_MESSAGE     7
#define LCD_TAB_LENGTH      8

void fifo_wait() {
  while(lcd05::read_fifo_length(LCD05_I2C_ADDRESS)<4);
}

void top_left_corner(char *msg, int length) {
  if(length > MAX_LCD_MESSAGE) {
    Serial.print("Top left corner message too long: ");
    Serial.println(msg);
    return;
  }

  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void top_right_corner(char *msg, int length) {
  if(length > MAX_LCD_MESSAGE) {
    Serial.print("Top left corner message too long: ");
    Serial.println(msg);
    return;
  }

  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::cursor_horizontal_tab(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void bottom_left_corner(char *msg, int length) {
  if(length > MAX_LCD_MESSAGE) {
    Serial.print("Top left corner message too long: ");
    Serial.println(msg);
    return;
  }

  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::cursor_vertical_tab(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void bottom_right_corner(char *msg, int length) {
  if(length > MAX_LCD_MESSAGE) {
    Serial.print("Top left corner message too long: ");
    Serial.println(msg);
    return;
  }

  fifo_wait();

  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::cursor_vertical_tab(LCD05_I2C_ADDRESS);
  lcd05::cursor_horizontal_tab(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS, msg, length);
}

void setup()
{
  Serial.begin(SERIAL_BAUDS);
  Wire.begin();
  delay(LCD05_I2C_INIT_DELAY);

  Serial.print("initializing LCD05 display in address 0x");
  Serial.print(LCD05_I2C_ADDRESS<<1,HEX); Serial.println(" ...");

  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::set_tab_length(LCD05_I2C_ADDRESS,LCD_TAB_LENGTH);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::hide_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);
}

int counter = 0;

void loop()
{
  char corners[4][8];

  sprintf(
      corners[counter%4],
      "C%i: %i",
      counter%4, counter
      );

  switch(counter%4) {
  case 0:
    top_left_corner(corners[0],strlen(corners[0]));
    break;
  case 1:
    top_right_corner(corners[1],strlen(corners[1]));
    break;
  case 2:
    bottom_left_corner(corners[2],strlen(corners[2]));
    break;
  case 3:
    bottom_right_corner(corners[3],strlen(corners[3]));
    break;
  default:;
  }

  counter++;
  delay(1050);
}

