#include <Wire.h>

#define I2C_SLAVE_ADDR   0x26              // I2C Dust slave address

#define DELAY_DUST_READ    80              // wait 80 ms before ask dust value
#define DELAY_NEW_SAMPLE 1000              // time before new sample (1s)

float dustValue;
byte* floatPtr;

void setup(){
  Wire.begin();                            // start I2C Bus as Master
  Serial.begin(38400);                     // start serial to send dust output
  dustValue = 0;
}

void loop(){
  dustValue = 0;
  
  Wire.beginTransmission(I2C_SLAVE_ADDR);  // transmit to slave device
  Wire.write(0xFF);                        // sends usuless data 
  Wire.endTransmission();                  // stop transmitting

  delay(DELAY_DUST_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &dustValue;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read();
	Serial.println(dustValue,DEC);     // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }

  delay(DELAY_NEW_SAMPLE);
}