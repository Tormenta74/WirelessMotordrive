/* lcd05.h
 * Example showing how to use the Devantech LCD05 LCD 16x2 display
 * in I2C mode. More info about this LCD display in
 *   http://www.robot-electronics.co.uk/htm/Lcd05tech.htm
 * author: Antonio C. Domínguez Brito <adominguez@iusiani.ulpgc.es>
 */

#ifndef lcd05_h
#define lcd05_h

#include <Arduino.h>

#define LCD05_I2C_INIT_DELAY 100 // in milliseconds

// LCD05's command related definitions
#define COMMAND_REGISTER byte(0x00)
#define FIFO_AVAILABLE_LENGTH_REGISTER byte(0x00)
#define LCD_STYLE_16X2 byte(5)

// LCD05's command codes
#define CURSOR_HOME             byte(1)
#define SET_CURSOR              byte(2) // specify position with a byte in the interval 0-32/80
#define SET_CURSOR_COORDS       byte(3) // specify position with two bytes, line and column
#define HIDE_CURSOR             byte(4)
#define SHOW_UNDERLINE_CURSOR   byte(5)
#define SHOW_BLINKING_CURSOR    byte(6)
#define BACKSPACE               byte(8)
#define HORIZONTAL_TAB          byte(9) // advances cursor a tab space
#define SMART_LINE_FEED         byte(10) // moves the cursor to the next line in the same column
#define VERTICAL_TAB            byte(11) // moves the cursor to the previous line in the same column
#define CLEAR_SCREEN            byte(12)
#define CARRIAGE_RETURN         byte(13)
#define CLEAR_COLUMN            byte(17)
#define TAB_SET                 byte(18) // specify tab size with a byte in the interval 1-10
#define BACKLIGHT_ON            byte(19)
#define BACKLIGHT_OFF           byte(20) // this is the default
#define DISABLE_STARTUP_MESSAGE byte(21)
#define ENABLE_STARTUP_MESSAGE  byte(22)
#define SAVE_AS_STARTUP_SCREEN  byte(23)
#define SET_DISPLAY_TYPE        byte(24) // followed by the type, which is byte 5 for a 16x2 LCD style
#define CHANGE_ADDRESS          byte(25) // see LCD05 specification
#define CUSTOM_CHAR_GENERATOR   byte(27) // see LCD05 specification
#define DOUBLE_KEYPAD_SCAN_RATE byte(28)
#define NORMAL_KEYPAD_SCAN_RATE byte(29)
#define CONTRAST_SET            byte(30) // specify contrast level with a byte in the interval 0-255
#define BRIGHTNESS_SET          byte(31) // specify brightness level with a byte in the interval 0-255


namespace lcd05 {
  void set_display_type(byte address, byte type);
  void clear_screen(byte address);
  void cursor_home(byte address);
  void set_cursor(byte address, byte pos);
  void set_cursor_coords(byte address, byte line, byte column);
  void show_blinking_cursor(byte address);
  void backlight_on(byte address);
  void backlight_off(byte address);
  bool ascii_chars(byte address, char* bytes, int length);
  byte read_fifo_length(byte address);
}

#endif
