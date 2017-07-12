
BOARD_TAG         = uno
ARDUINO_PORT	  = /dev/ttyACM0
#ARDUINO_PORT	  = /dev/ttyACM1
MONITOR_BAUDRATE  = 38400

CXXFLAGS_STD      = -std=c++11

LDFLAGS			  += -Wl,-u,vfprintf -lprintf_flt

ARDUINO_LIBS	  = Time Xbee lcd05 rd02 Wire

include $(ARDMK_DIR)/Arduino.mk
