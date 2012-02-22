/*
                  +-\/-+
        (RESET)  1|    |8  VCC (2.7-5.5V)
      Temp Read  2|    |7  I2C   SCK -> Uno A5
          DEBUG  3|    |6  (PB1) NA
            GND  4|    |5  I2C   SDA -> Uno A4
                  +----+ 
*/

#include "TinyWireS.h"      // ATTiny wire lib
#include "Statistic.h"      // Arduino Statistic lib

#define I2C_SLAVE_ADDR 0x27 // I2C slave address

#define TEMP_READ        3  // Temperature read on pin 2 (ADC3)
//#define DEBUG          PB4  // Temperature read on pin 2 (ADC3)

#define DELAY_ADC       10  // time before re-read ADC 10 us

Statistic temp;
float     result;

byte*     floatPtr;
byte      byteRcvd;
byte      i;

void setup() {
  pinMode(TEMP_READ,INPUT);
//pinMode(DEBUG,OUTPUT);

  TinyWireS.begin(I2C_SLAVE_ADDR);

  temp.clear();
  result  = 0;
  byteRcvd= 0;
  i       = 0;
}

void loop() {
    if (TinyWireS.available()){          // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // do nothing with the message
    
    for(i=0;i<50;i++) {                  // make 50 temperature sample
      temp.add(analogRead(TEMP_READ));   // read and store temperature sensor output
      delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
//    digitalWrite(DEBUG,HIGH);
//    digitalWrite(DEBUG,LOW);
    }

    result = ((temp.average()*0.0049)-0.5)*100 ; // convert the result into a humain readable output

    if(result < 0) result = 0;           // avoid eratic datas
    temp.clear();                        // clear statistics to avoid leack and data stacking
    
    floatPtr = (byte*) &result;
                TinyWireS.send( *floatPtr );  // send first byte
    ++floatPtr; TinyWireS.send( *floatPtr );  // the second
    ++floatPtr; TinyWireS.send( *floatPtr );  // third
    ++floatPtr; TinyWireS.send( *floatPtr );  // fourth and final byte
  }
}
