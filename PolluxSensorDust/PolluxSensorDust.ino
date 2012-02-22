/*
             +-\/-+
   (RESET)  1|    |8  VCC (2.7-5.5V)
 Dust Read  2|    |7  I2C   SCK -> Uno A5
  Dust Led  3|    |6  (PB1) NA
       GND  4|    |5  I2C   SDA -> Uno A4
             +----+ 
*/

#include "TinyWireS.h"      // ATTiny wire lib
#include "Statistic.h"      // Arduino Statistic lib

#define I2C_SLAVE_ADDR 0x26 // I2C slave address

#define DUST_READ        3  // Dust read on pin 2 (ADC3)
#define DUST_LED       PB4  // Dust LED control on pin 3 (PB4)

#define DELAY_TIME1     28  // Wait 28 us before read sensor value (cf : datasheet)
#define DELAY_TIME2      9  // Wait 10 ms before reread sensor value (cf : datasheet)

Statistic dust;
float     result;

byte*     floatPtr;
byte      byteRcvd;
byte      i;

void setup() {
  pinMode(DUST_LED,OUTPUT);
  pinMode(DUST_READ,INPUT);

  TinyWireS.begin(I2C_SLAVE_ADDR);

  dust.clear();
  result  = 0;
  byteRcvd= 0;
  i       = 0;
}

void loop() {
    if (TinyWireS.available()){          // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // do nothing with the message
    
    for(i=0;i<=5;i++) {                  // make 6 dust sample
      digitalWrite(DUST_LED,LOW);        // power on the LED
      delayMicroseconds(DELAY_TIME1);    // for 28 ms
      int tmp = analogRead(DUST_READ);   // read and store the dust sensor output
      digitalWrite(DUST_LED,HIGH);       // turn the LED off
      dust.add(float(tmp));              // add the sensor value in statistics (avoid CPU consumption during timer)
      delay(DELAY_TIME2);                // way 9ms to reach the 10 ms until new sample
    }
    result = ((dust.average()*0.0049)-0.5)*0.18 ; // convert the result into a humain readable output
    if(result < 0) result = 0;           // avoid eratic datas
    dust.clear();                        // clear statistics to avoid leack and data stacking
    
    floatPtr = (byte*) &result;
                TinyWireS.send( *floatPtr );  // send first byte
    ++floatPtr; TinyWireS.send( *floatPtr );  // the second
    ++floatPtr; TinyWireS.send( *floatPtr );  // third
    ++floatPtr; TinyWireS.send( *floatPtr );  // fourth and final byte
  }
}
