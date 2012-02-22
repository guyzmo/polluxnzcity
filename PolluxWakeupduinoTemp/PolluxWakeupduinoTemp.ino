#include <Wire.h>

#define I2C_SLAVE_ADDR_TEMP   0x27         // I2C Temperature slave address

#define DELAY_TEMP_READ   140              // wait 140 ms before ask temp value
#define DELAY_NEW_SAMPLE 1000              // time before new sample (1s)

float tempValue;
byte* floatPtr;

void setup(){
  Wire.begin();                            // start I2C Bus as Master
  Serial.begin(38400);                     // start serial to send dust output
  tempValue = 0;
}

void loop(){
  tempValue = 0;
  
  Wire.beginTransmission(I2C_SLAVE_ADDR_TEMP);  // transmit to slave device
  Wire.write(0xFF);                             // sends usuless data 
  Wire.endTransmission();                       // stop transmitting

  delay(DELAY_TEMP_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR_TEMP, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &tempValue;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read(); ++floatPtr;
	(*floatPtr) = Wire.read();
	Serial.print("Temp = ");      // print out slave byte to Serial monitor
	Serial.print(tempValue,DEC);  // print out slave byte to Serial monitor
	Serial.println(" C");         // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }

  delay(DELAY_NEW_SAMPLE);
}
