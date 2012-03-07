/*
             +-\/-+
   (RESET)  1|    |8  VCC (2.7-5.5V)
   NA ADC3  2|    |7  I2C   SCK -> Uno A5
       FAN  3|    |6  (PB1) NA
       GND  4|    |5  I2C   SDA -> Uno A4
             +----+ 
*/

#include "TinyWireS.h"      // ATTiny wire lib

#define I2C_SLAVE_ADDR 0x26 // I2C slave address

#define FAN_RMT        PB3  // Dust LED control on pin 3 (PB4)
#define DELAY_BASE     100  // basic delay 100 ms

byte byteRcvd ;
byte i;

void setup() {
  pinMode(FAN_RMT,OUTPUT);
  TinyWireS.begin(I2C_SLAVE_ADDR);
  i = 0;
  byteRcvd = 0;
}

void loop() {
  /*if (TinyWireS.available()) {           // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // do nothing with the message
    digitalWrite(FAN_RMT, HIGH);         // set the LED on
    for(i=0;i<byteRcvd;i++)
      delay(DELAY_BASE);
    digitalWrite(FAN_RMT, LOW);          // set the LED on
  }*/
  if (TinyWireS.available()) {           // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // do nothing with the message
    digitalWrite(FAN_RMT, HIGH);         // set the LED on
    delay(DELAY_BASE);
    digitalWrite(FAN_RMT, LOW);          // set the LED on
  }
}
