/*
    ATTiny85 slave, sample a sensor and send a float value to Arduino UNO master
    Copyright (C) <2012>  <Lucas Fernandez>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


                  +-\/-+
        (RESET)  1|    |8  VCC (2.7-5.5V)
       SPL Read  2|    |7  I2C   SCK -> Uno A5
      Temp Read  3|    |6  Fan RMT
            GND  4|    |5  I2C   SDA -> Uno A4
                  +----+ 
*/

#include "TinyWireS.h"      // ATTiny wire lib
#include "Statistic.h"      // Arduino Statistic lib

#define I2C_SLAVE_ADDR 0x27 // I2C slave address

#define TEMP_READ        2  // Temp  read on pin 3 (ADC2)
#define  SPL_READ        3  // SPL   read on pin 2 (ADC3)
#define  FAN_RMT       PB1  // Fan output on pin 6 (PB1)

#define DELAY_ADC       10  // time before re-read ADC 10 us

Statistic temp;
float     result;

byte*     floatPtr;
byte      byteRcvd;
byte      i;

// sensor  dbmeter
// 680  == 345 == 90dB
// 672  == 330 == 85dB
// 620  == 415 == 80dB
// 450? == 397 == 75dB
// 420? == 390 == 70dB
// 270  == 331 == 66dB

byte convert_to_dB(float val) {
    // 10.0 == 50dB
    if (val<10.0)        return 50; // <50dB
    // 15.0 == 55dB
    else if (val<15.0)   return 55; // <55dB
    // 30.0 == 60dB
    else if (val<30.0)   return 60; // <60dB
    // 70.0 == 65dB
    else if (val<70.0)   return 65; // <65dB
    // 140.0 == 70dB
    else if (val<140.0)  return 70; // <70dB
    // 230.0 == 75dB
    else if (val<230.0)  return 75; // <75dB
    // 250.0 == 80dB
    else if (val<255.0)  return 80; // <80dB
    // 255.0 == 85dB == 90 dB
    else if (val<255.0)  return 85; // <85dB
    else if (val>=255.0) return 90; // >85dB
}

void setup() {
  pinMode(TEMP_READ,INPUT);
  pinMode(SPL_READ,INPUT);
  pinMode(FAN_RMT,OUTPUT);
  
  digitalWrite(FAN_RMT,LOW);

  TinyWireS.begin(I2C_SLAVE_ADDR);

  temp.clear();
  result  = 0;
  byteRcvd= 0;
  i       = 0;
}

void loop() {
    if (TinyWireS.available()){          // if we get an I2C message
    byteRcvd = TinyWireS.receive();      // doqq nothing with the message
    
    if(byteRcvd == 0xAA) {
    for(i=0;i<50;i++) {                  // make 50 temperature sample
      temp.add(analogRead(TEMP_READ));   // read and store temperature sensor output
      delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
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
    
        
    if(byteRcvd == 0xBB) {
    digitalWrite(FAN_RMT,HIGH);
    delay(250);
    digitalWrite(FAN_RMT,LOW);
    }
    
    if(byteRcvd == 0xCC) {
    for(i=0;i<50;i++) {                  // make 50 temperature sample
      temp.add(analogRead(SPL_READ));   // read and store temperature sensor output
      delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
    }
    
    byte spl = (byte)convert_to_dB(temp.average());
    temp.clear();

    TinyWireS.send(spl);
    }
  }
}
