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

#include <Wire.h>
#include <XBee.h>
#include <lcd05.h>

#define LCD05_I2C_ADDRESS byte((0xC6)>>1)

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
ZBTxRequest tx=ZBTxRequest();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

#define MAX_ZB_PAYLOAD_LENGTH 72 // according to xbee-arduino documentation
uint8_t payload[MAX_ZB_PAYLOAD_LENGTH];

void setup() 
{
  // start serial
  Serial.begin(38400);
  xbee.begin(Serial); 
  Wire.begin();
  lcd05::set_display_type(LCD05_I2C_ADDRESS,LCD_STYLE_16X2);
  lcd05::clear_screen(LCD05_I2C_ADDRESS);
  lcd05::cursor_home(LCD05_I2C_ADDRESS);
  lcd05::show_blinking_cursor(LCD05_I2C_ADDRESS);
  lcd05::backlight_on(LCD05_I2C_ADDRESS);
}

// continuously reads packets, looking for ZB Receive or Modem Status
void loop() 
{    
  int i;

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
      lcd05::ascii_chars(
          LCD05_I2C_ADDRESS,
          (char*)payload,
          min(MAX_ZB_PAYLOAD_LENGTH,rx.getDataLength())
          );

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
