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
 Dust Read  2|    |7  I2C   SCK -> Uno A5
  Dust Led  3|    |6  (PB1) NA
       GND  4|    |5  I2C   SDA -> Uno A4
             +----+ 
*/
#include <Statistic.h>
#include <TinyWire.h>

#define I2C_SLAVE_ADDR 0x26 // I2C slave address

#define DUST_READ        3  // Dust read on pin 2 (ADC3)
#define DUST_LED       PB4  // Dust LED control on pin 3 (PB4)

#define DELAY_TIME1     28  // Wait 28 us before read sensor value (cf : datasheet)
#define DELAY_TIME2      9  // Wait 10 ms before reread sensor value (cf : datasheet)

TinyWire Wire;

prog_uint32_t offset=(float)-0.5;
prog_uint32_t coef=(float)0.18;

void sensor_calibrate() {
    offset = Wire.read(0x20);
    offset = Wire.read(0x21);
    offset = Wire.read(0x22);
    offset = Wire.read(0x23);
}

void sensor_meas() {
    Statistic stat;
    float result;
    uint8_t* float_ptr;

    for(uint8_t i=0;i<=5;i++) {            // make 6 dust sample
        digitalWrite(DUST_LED,LOW);        // power on the LED
        delayMicroseconds(DELAY_TIME1);    // for 28 ms
        int tmp = analogRead(DUST_READ);   // read and store the dust sensor output
        digitalWrite(DUST_LED,HIGH);       // turn the LED off
        stat.add(float(tmp));              // add the sensor value in statistics (avoid CPU consumption during timer)
        delay(DELAY_TIME2);                // way 9ms to reach the 10 ms until new sample
    }

    result = ((stat.average()*0.0049)+offset)*coef ; // convert the result into a humain readable output
    if(result < 0) result = 0;             // avoid eratic datas
    stat.clear();                          // clear statistics to avoid leack and data stacking

    float_ptr = (uint8_t*)&result;

    Wire.write(0x11,*(float_ptr)); // send 1st byte of float
    Wire.write(0x12,*(float_ptr+1)); // send 2nd byte of float
    Wire.write(0x13,*(float_ptr+2)); // send 3rd byte of float
    Wire.write(0x14,*(float_ptr+3)); // send 4th byte of float
}

void setup() {
    pinMode(DUST_LED,OUTPUT);
    pinMode(DUST_READ,INPUT);

    Wire.begin(I2C_SLAVE_ADDR); // slave address
}


void loop() {
    // on I2C bus : (addr=0x26,reg=0x00,val=1)
    if (Wire.read(0x00) != 0)
        sensor_meas();

    // on I2C bus : (addr=0x26,reg=0x10)
    if (Wire.read(0x01) != 0)
        sensor_calibrate();
}

