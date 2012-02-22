#include <Wire.h>

#define I2C_SLAVE_ADDR   0x28              // I2C Sound slave address

#define DELAY_SOUND_READ   140             // wait 140 ms before ask dust value
#define DELAY_NEW_SAMPLE 1000              // time before new sample (1s)

byte soundValue;

void setup(){
  Wire.begin();                            // start I2C Bus as Master
  Serial.begin(38400);                     // start serial to send dust output
  soundValue = 0;
}

void loop(){
  soundValue = 0;
  
  Wire.beginTransmission(I2C_SLAVE_ADDR);  // transmit to slave device
  Wire.write(0xFF);                        // sends usuless data 
  Wire.endTransmission();                  // stop transmitting

  delay(DELAY_SOUND_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR, 1);     // request 1 byte from slave

  if (Wire.available()) {
    soundValue = Wire.read();
	Serial.print("Sound = ");              // print out slave byte to Serial monitor
	Serial.print(soundValue,DEC);          // print out slave byte to Serial monitor
	Serial.println(" dB");                 // print out slave byte to Serial monitor
  } else {
    Serial.println("NaN");  
  }

  delay(DELAY_NEW_SAMPLE);
}
