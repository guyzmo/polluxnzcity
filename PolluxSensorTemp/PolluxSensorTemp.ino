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
 Temp Read  2|    |7  I2C   SCK -> Uno A5
        NA  3|    |6  (PB1) NA
       GND  4|    |5  I2C   SDA -> Uno A4
             +----+ 
*/
#include <Statistic.h>
#include <TinyWire.h>

#define I2C_SLAVE_ADDR 0x25 // I2C slave address

#define TEMP_READ        3  // Dust read on pin 2 (ADC3)
#define DELAY_TIME      10  // Wait 10 ms before reread sensor value
#define NB_READ        100

TinyWire Wire;

prog_uint32_t offset=(float)-0.5;

void sensor_calibrate() {
    offset = Wire.read(0x20);
    offset = Wire.read(0x21);
    offset = Wire.read(0x22);
    offset = Wire.read(0x23);
}

void sensor_meas() {
    Statistic stat;
    float     result;
    uint8_t*  float_ptr;

    for(uint8_t i=0;i<NB_READ;i++) {            // make 100 temp sample
        stat.add(float(analogRead(TEMP_READ))); // add the sensor value in statistics
        delay(DELAY_TIME);                      // wait 1ms to lets ADC converge
    }

    result = (stat.average()*0.0049)/62.9 ;
    result = result + 0.556792873;
    result = 8980 * result;
    result = 35678500 / result;
    result = result - 4715 ;

    if(result < 0) result = 0;             // avoid eratic datas
    stat.clear();                          // clear statistics to avoid leack and data stacking

    float_ptr = (uint8_t*)&result;

    Wire.write(0x11,*(float_ptr));   // send 1st byte of float
    Wire.write(0x12,*(float_ptr+1)); // send 2nd byte of float
    Wire.write(0x13,*(float_ptr+2)); // send 3rd byte of float
    Wire.write(0x14,*(float_ptr+3)); // send 4th byte of float
}

void setup() {
    pinMode(TEMP_READ,INPUT);
    Wire.begin(I2C_SLAVE_ADDR); // slave address
}


void loop() {
    // on I2C bus : (addr=0x25,reg=0x00,val=1)
    if (Wire.read(0x00) != 0)
        sensor_meas();

    // on I2C bus : (addr=0x26,reg=0x10)
    if (Wire.read(0x01) != 0)
        sensor_calibrate();
}

