/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 *
 * Pollux'NZ City is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pollux'NZ City is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project. If not, see <http://www.gnu.org/licenses/>.

                  +-\/-+
        (RESET)  1|    |8  VCC (2.7-5.5V)
            SPL  2|    |7  I2C   SCK -> Uno A5
      Temp Read  3|    |6  Fan Remote
            GND  4|    |5  I2C   SDA -> Uno A4
                  +----+ 
*/
#define I2C_SLAVE_ADDR 0x27 // I2C slave address

//#define DESC "Temperature sensor"
#include <TinyWire.h>

#include "Statistic/Statistic.h"      // Arduino Statistic lib

#define TEMP_READ      2    // Temp  read on pin 3 (ADC2)
#define SPL_READ       3    // SPL   read on pin 2 (ADC3)
#define FAN_RMT        PB1  // Fan output on pin 6 (PB1)

#define DELAY_ADC       10  // time before re-read ADC 10 us

TinyWire Wire;


////////////////////////////////////////////////////////////////////////////////

// sensor  dbmeter
// 680  == 345 == 90dB
// 672  == 330 == 85dB
// 620  == 415 == 80dB
// 450? == 397 == 75dB
// 420? == 390 == 70dB
// 270  == 331 == 66dB
inline byte convert_to_dB(float val) {
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
    else //if (val>=255.0)
        return 90; // >85dB
}

byte get_spl(Statistic* stat) {   
    byte result;

    for(uint8_t i=0;i<50;i++) {            // make 50 temperature sample
        stat->add(analogRead(SPL_READ));  // read and store temperature sensor output
        delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
    }

    result = (byte)convert_to_dB(stat->average());
    stat->clear();

    return result;
}

float get_temp(Statistic* stat) {
    float result;
    for(uint8_t i=0;i<50;i++) {            // make 50 temperature sample
        stat->add(analogRead(TEMP_READ));        // read and store temperature sensor output
        delayMicroseconds(DELAY_ADC);      // wait 10 us to stabilise ADC
    }

    result = ((stat->average()*0.0049)-0.5125)*100 ; // convert the result into a humain readable output

    if(result < 0) result = 0;             // avoid eratic datas
    stat->clear();                         // clear statistics to avoid leack and data stacking

    return result;
}

void run_fan() {
    digitalWrite(PB1,HIGH);
    for (int i=0;i<10;++i)
        delay(10000);
    digitalWrite(PB1,LOW);
}

////////////////////////////////////////////////////////////////////////////////
uint8_t i=0;

void sensor_meas() {
    Statistic stat;
    float res_f = 0;
    uint8_t res_b = 0;

    uint8_t* float_ptr;

    switch (i++) {
        case 0:
            run_fan();
            Wire.set_type(I2C_INT);
            Wire.write((uint8_t)0xFF); // msb
            Wire.write((uint8_t)0xFF); // lsb
            break;

        case 1:
            res_b = get_spl(&stat);
            Wire.set_type(I2C_INT);
            Wire.write((uint8_t)res_b); // msb
            Wire.write((uint8_t)0x0);   // lsb
            break;
            
        case 2:
            res_f = get_temp(&stat);
            float_ptr = (uint8_t*)&res_f;
            Wire.set_type(I2C_FLT);
            Wire.write(*(float_ptr)); // send 1st byte of float
            Wire.write(*(float_ptr+1)); // send 2nd byte of float
            Wire.write(*(float_ptr+2)); // send 3rd byte of float
            Wire.write(*(float_ptr+3)); // send 4th byte of float
            break;
    }
    if (i == 3) i = 0;
}

void setup() {
    pinMode(PB1,OUTPUT);
    pinMode(TEMP_READ,INPUT);
    pinMode(SPL_READ,INPUT);

    Wire.begin(I2C_SLAVE_ADDR); // slave address
    Wire.set_request_callback(&sensor_meas);
}

void loop() {
 /* nop */
}

