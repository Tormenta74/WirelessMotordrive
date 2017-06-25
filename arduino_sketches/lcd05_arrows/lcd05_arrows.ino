#include <lcd05.h>
#include <Wire.h>

#define SERIAL_BAUDS        9600

#define LCD05_I2C_ADDRESS   byte((0xC6)>>1)
#define MAX_LCD_MESSAGE     7
#define LCD_TAB_LENGTH      8

#define LUP_ARROW_CHAR_POS    128
#define UP_ARROW_CHAR_POS     129
#define RUP_ARROW_CHAR_POS    130
#define RDOWN_ARROW_CHAR_POS  131
#define DOWN_ARROW_CHAR_POS   132
#define LDOWN_ARROW_CHAR_POS  133

#define SPECIAL_CHARS_NUM     6

// bitmaps
#define CHAR_LINES       8
#define CHAR_LINE_LENGTH 8

const char ULA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10011110,
  0b10011000,
  0b10010100,
  0b10010010,
  0b10000001,
  0b10000000,
  0b10000000
};

const char UA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10000100,
  0b10001110,
  0b10010101,
  0b10000100,
  0b10000100,
  0b10000100,
  0b10000000
};


const char URA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10001111,
  0b10000011,
  0b10000101,
  0b10001001,
  0b10010000,
  0b10000000,
  0b10000000
};

const char DRA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10000000,
  0b10010000,
  0b10001001,
  0b10000101,
  0b10000011,
  0b10001111,
  0b10000000
};

const char DA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10000100,
  0b10000100,
  0b10000100,
  0b10010101,
  0b10001110,
  0b10000100,
  0b10000000
};

const char DLA_BYTES[CHAR_LINES] = {
  0b10000000,
  0b10000000,
  0b10000001,
  0b10010010,
  0b10010100,
  0b10011000,
  0b10011110,
  0b10000000
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
  delay(LCD05_I2C_INIT_DELAY);

  // up arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],UA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,UP_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  // up rigth arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],URA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,RUP_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  // down rigth arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DRA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,RDOWN_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  // down arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,DOWN_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  // down left arrow
  for(int i=0; i<CHAR_LINE_LENGTH; i++) {
    memcpy(bytes_array[i],DLA_BYTES+i,CHAR_LINE_LENGTH);
  }
  lcd05::add_special_char(LCD05_I2C_ADDRESS,bytes_array,LDOWN_ARROW_CHAR_POS);
  delay(LCD05_I2C_INIT_DELAY);

  for(int i=0; i<8; i++)
    if(bytes_array[i]) free(bytes_array[i]);
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

char holder[SPECIAL_CHARS_NUM+2];

void setup()
{
  Serial.begin(SERIAL_BAUDS);
  Wire.begin();
  delay(LCD05_I2C_INIT_DELAY);

  Serial.print("initializing LCD05 display in address 0x");
  Serial.print(LCD05_I2C_ADDRESS<<1,HEX); Serial.println(" ...");

  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::set_tab_length(LCD05_I2C_ADDRESS,LCD_TAB_LENGTH);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::hide_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);

  Serial.print("adding special characters...");
  add_arrow_chars();
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  Serial.print("done.");

  holder[0] = LUP_ARROW_CHAR_POS;
  holder[1] = UP_ARROW_CHAR_POS;
  holder[2] = RUP_ARROW_CHAR_POS;
  holder[3] = 0b01111110;
  holder[4] = RDOWN_ARROW_CHAR_POS;
  holder[5] = DOWN_ARROW_CHAR_POS;
  holder[6] = LDOWN_ARROW_CHAR_POS;
  holder[7] = 0b01111111;
}

int counter = 0;

void loop()
{
  char buffer[14];
  sprintf(buffer,"char %i (%i) ",counter,holder[counter]);

  fifo_wait();
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,buffer,14);
  lcd05::ascii_chars(LCD05_I2C_ADDRESS,holder+counter,1);
  counter++;
  counter%=8;
  delay(3000);
}

