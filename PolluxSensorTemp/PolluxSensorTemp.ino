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

float convert_resistor_to_temp(float resitor) {
  float ref_temp[] = {-40 ,-30 ,-20 ,-10 ,0   ,10  ,20  ,30  ,40  ,50  ,60  ,70  ,80  ,90};
  float ref_res[]  = {1584,1649,1715,1784,1854,1926,2000,2076,2153,2233,2314,2397,2482,2569};
  short len = 14;
  
  for (short i;i<len;i++)
    if(ref_res[i] >= resitor) {
      float range = ref_res[i] - ref_res[i-1];
      float delta = resitor - ref_res[i-1];
      return ref_temp[i-1]+((delta/range)*10);
    }
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

    result = convert_resistor_to_temp(result);

    float_ptr = (uint8_t*)&result;
    Wire.set_type(I2C_FLT);
    Wire.write(*(float_ptr));   // send 1st byte of float
    Wire.write(*(float_ptr+1)); // send 2nd byte of float
    Wire.write(*(float_ptr+2)); // send 3rd byte of float
    Wire.write(*(float_ptr+3)); // send 4th byte of float
}

void setup() {
    pinMode(TEMP_READ,INPUT);
    Wire.begin(I2C_SLAVE_ADDR); // slave address
    Wire.set_request_callback(&sensor_meas);
}


void loop() {
    /* nop */
}

