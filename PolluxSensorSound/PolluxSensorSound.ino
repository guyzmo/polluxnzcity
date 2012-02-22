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

const char* convert_to_dB_ascii(float val) {
    // 10.0 == 50dB
    if (val<11.0)        return "  <50dB"; // <50dB
    // 15.0 == 55dB
    else if (val<16.0)   return "50-55dB"; // <55dB
    // 30.0 == 60dB
    else if (val<31.0)   return "55-60dB"; // <60dB
    // 70.0 == 65dB
    else if (val<71.0)   return "60-65dB"; // <65dB
    // 140.0 == 70dB
    else if (val<141.0)  return "65-70dB"; // <70dB
    // 230.0 == 75dB
    else if (val<230.0)  return "70-75dB"; // <75dB
    // 250.0 == 80dB
    else if (val<251.0)  return "75-80dB"; // <80dB
    // 255.0 == 85dB == 90 dB
    else if (val<256.0)  return "80-85dB"; // <85dB
    // and over...
    else if (val>=256.0) return "  >85dB"; // >85dB
}

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
      sound.add(analogRead(SOUND_READ)); // read and store temperature sensor output
      delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
      digitalWrite(DEBUG,HIGH);
      digitalWrite(DEBUG,LOW);
    }

    TinyWireS.send(convert_to_dB(sound.average()));
    sound.clear();                        // clear statistics to avoid leack and data stacking
  }
}
