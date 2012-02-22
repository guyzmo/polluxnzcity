/*
              +-\/-+
    (RESET)  1|    |8  VCC (2.7-5.5V)
 Sound Read  2|    |7  I2C   SCK -> Uno A5
      DEBUG  3|    |6  (PB1) NA
        GND  4|    |5  I2C   SDA -> Uno A4
              +----+ 
*/

#include "TinyWireS.h"       // ATTiny wire lib
#include "Statistic.h"       // Arduino Statistic lib

#define I2C_SLAVE_ADDR 0x28  // I2C slave address

#define SOUND_READ        3  // SPL read on pin 2 (ADC3)
#define DEBUG           PB4  // DEBUG output on pin 3 (PB4)

#define DELAY_ADC        10  // Wait 10 us before reread sensor value (cf : datasheet)

Statistic sound;
float     result;

byte*     floatPtr;
byte      byteRcvd;
byte      i;

void setup() {
  pinMode(DEBUG,OUTPUT);
  pinMode(SOUND_READ,INPUT);

  TinyWireS.begin(I2C_SLAVE_ADDR);

  sound.clear();
  result  = 0;
  byteRcvd= 0;
  i       = 0;
}

void loop() {
    if (TinyWireS.available()){          // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // do nothing with the message
    
    for(i=0;i<50;i++) {                  // make 50 temperature sample
      sound.add(analogRead(SOUND_READ));   // read and store temperature sensor output
      delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
//    digitalWrite(DEBUG,HIGH);
//    digitalWrite(DEBUG,LOW);
    }

    result = ((sound.average()*0.0049)-0.5)*100 ; // convert the result into a humain readable output

    if(result < 0) result = 0;           // avoid eratic datas
    sound.clear();                        // clear statistics to avoid leack and data stacking
    
    floatPtr = (byte*) &result;
                TinyWireS.send( *floatPtr );  // send first byte
    ++floatPtr; TinyWireS.send( *floatPtr );  // the second
    ++floatPtr; TinyWireS.send( *floatPtr );  // third
    ++floatPtr; TinyWireS.send( *floatPtr );  // fourth and final byte
  }
}