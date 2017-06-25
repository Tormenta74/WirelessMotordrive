#include <lcd05.h>
#include <Wire.h>

#define LCD05_I2C_ADDRESS byte((0xC6)>>1)

bool backlight=false;

void setup()
{
  Serial.begin(9600);

  Serial.println("initializing Wire interface ...");
  Wire.begin();
  delay(LCD05_I2C_INIT_DELAY);

  Serial.print("initializing LCD05 display in address 0x");
  Serial.print(LCD05_I2C_ADDRESS,HEX); Serial.println(" ...");

  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::show_blinking_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_off(LCD05_I2C_ADDRESS);
}

void loop()
{
  int fifo_length;
  while( (fifo_length=lcd05::read_fifo_length(LCD05_I2C_ADDRESS))<4 ) {
    //do nothing
  }
  Serial.print("fifo length: "); Serial.println(fifo_length);

  lcd05::ascii_chars(LCD05_I2C_ADDRESS,(byte*)("hi"),4);

  backlight=!backlight;
  if(backlight) lcd05::backlight_off(LCD05_I2C_ADDRESS);
  else lcd05::backlight_on(LCD05_I2C_ADDRESS);

  delay(1000);
}
