
#include <Wire.h>
#include <rd02.h>

#define RD02_I2C_ADDRESS  byte((0xB0)>>1)

int32_t encoder1 = 0;
int32_t encoder2 = 0;

void setup() {
  int version = -1;
  Serial.begin(9600);
  Wire.begin();
  rd02::reset_encoders(RD02_I2C_ADDRESS);
  rd02::disable_speed_regulation(RD02_I2C_ADDRESS);
  rd02::set_mode(RD02_I2C_ADDRESS, 1);
  rd02::set_speed1(RD02_I2C_ADDRESS, 0);
  rd02::set_speed2(RD02_I2C_ADDRESS, 0);
  version = rd02::get_software_rev(RD02_I2C_ADDRESS);
  Serial.println("rd02 module initialized in mode 1");
  Serial.print("software version "); Serial.println(version);
  encoder1 = rd02::read_enc1(RD02_I2C_ADDRESS);
  encoder2 = rd02::read_enc2(RD02_I2C_ADDRESS);
  delay(1000);
  Serial.println("now walking");
}

void loop() {
  rd02::set_speed1(RD02_I2C_ADDRESS, 32-1);
  rd02::set_speed2(RD02_I2C_ADDRESS, 16-1);
  encoder1 = rd02::read_enc1(RD02_I2C_ADDRESS);
  encoder2 = rd02::read_enc2(RD02_I2C_ADDRESS);
  Serial.print("encoder 1 = "); Serial.println(encoder1,DEC);
  Serial.print("encoder 2 = "); Serial.println(encoder2,DEC);
  delay(500);
}
