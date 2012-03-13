/*
                  +-\/-+
        (RESET)  1|    |8  VCC (2.7-5.5V)
      Temp Read  2|    |7  I2C   SCK -> Uno A5
          DEBUG  3|    |6  (PB4) NA
            GND  4|    |5  I2C   SDA -> Uno A4
                  +----+ 
*/

#define I2C_SLAVE_ADDR 0x27 // I2C slave address

#define DESC "Temperature sensor"
#include <TinyWire.h>

#include "Statistic/Statistic.h"      // Arduino Statistic lib

#define TEMP_READ        3  // Temperature read on pin 2 (ADC3)
#define DELAY_ADC       10  // time before re-read ADC 10 us

TinyWire Wire;

void sensor_meas() {
    Statistic temp;
    float     result;
    
    temp.clear();
    result  = 0;

    for(uint8_t i=0;i<50;i++) {          // make 50 temperature sample
        temp.add(analogRead(TEMP_READ)); // read and store temperature sensor output
        delayMicroseconds(DELAY_ADC);    // wait 10 us to stabilise ADC
    }

    result = ((temp.average()*0.0049)-0.5)*100 ; // convert the result into a human readable output

    if(result < 0) result = 0;           // avoid eratic datas
    temp.clear();                        // clear statistics to avoid leack and data stacking

    Wire.set_type(I2C_FLT);
    Wire.write((uint8_t)(((int)result)&0xFF));       // LSB
    Wire.write((uint8_t)((((int)result)>>8)&0xFF));  // MSB
}

void setup() {
    Wire.begin(I2C_SLAVE_ADDR); // slave address
    Wire.set_request_callback(&sensor_meas);
}

void loop() {
 /* nop */
}

