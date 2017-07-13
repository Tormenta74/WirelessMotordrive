
BOARD_TAG         = uno
# change this if it connects to the wrong port
ARDUINO_PORT	  = /dev/ttyACM0
#ARDUINO_PORT	  = /dev/ttyACM1
MONITOR_BAUDRATE  = 38400

CXXFLAGS_STD      = -std=c++11

LDFLAGS			  += -Wl,-u,vfprintf -lprintf_flt

# you need to have the libraries in the proper place
# ( $ARDUINO_SKETCHBOOK_DIR/libraries/ )
ARDUINO_LIBS	  = Time Xbee lcd05 rd02 Wire

# other variables to consider (not set here, but system wide)
# ARDUINO_DIR = installation path
# ARDUINO_SKETCHBOOK
# ARDMK_DIR = Arduino-Makefile root path
# AVR_TOOLS_DIR="$ARDUINO_DIR/hardware/tools/avr"

include $(ARDMK_DIR)/Arduino.mk
