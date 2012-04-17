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
 (ADC3) NO2  2|    |7  I2C   SCK -> Uno A5
 (ADC2)  CO  3|    |6  (PB1) NA
        GND  4|    |5  I2C   SDA -> Uno A4
              +----+ 
*/
#include <Statistic.h>
#include <TinyWire.h>

#define I2C_SLAVE_ADDR 0x28 // I2C slave address

#define CO_PIN     2
#define NO2_PIN    3
#define ADC_DELAY  1
#define AOP_AMP    23.6
#define CO_COEF    132.41525
#define NO2_COEF   13.241525
int i ;

TinyWire Wire;

void sensor_meas() {
	Statistic stats;
    float result;
    uint8_t* float_ptr;
	
	stats.clear();
	result = 0;
	
	switch (i++) {
        case 0: /* **** CO ***************************************** */

        for(int i=0;i<5;i++) {
    	    stats.add(analogRead(CO_PIN));
    	    delayMicroseconds(ADC_DELAY);
        }

        result = (stats.average()*0.0045-(0.04*AOP_AMP))*CO_COEF ;
        if(result<0) result = 0;

    	stats.clear();
	    float_ptr = (uint8_t*)&result;
	    Wire.set_type(I2C_FLT);
	    Wire.write(*(float_ptr));   // send 1st byte of float
	    Wire.write(*(float_ptr+1)); // send 2nd byte of float
	    Wire.write(*(float_ptr+2)); // send 3rd byte of float
	    Wire.write(*(float_ptr+3)); // send 4th byte of float
		
		break;
		
		case 1: /* **** NO2 **************************************** */
	
    	for(int i=0;i<5;i++) {
      		stats.add(analogRead(NO2_PIN));
      		delayMicroseconds(ADC_DELAY);
    	}
	
    	result = (stats.average()*0.0045-(0.04*AOP_AMP))*NO2_COEF ;
    	if(result<0) result = 0;

	    float_ptr = (uint8_t*)&result;
	    Wire.set_type(I2C_FLT);
	    Wire.write(*(float_ptr));   // send 1st byte of float
	    Wire.write(*(float_ptr+1)); // send 2nd byte of float
	    Wire.write(*(float_ptr+2)); // send 3rd byte of float
	    Wire.write(*(float_ptr+3)); // send 4th byte of float

		break;
	}
	if(i==2) i = 0 ;
}

void setup(void) {
  pinMode(CO_PIN,INPUT);
  pinMode(NO2_PIN,INPUT);

  Wire.begin(I2C_SLAVE_ADDR); // slave address
  Wire.set_request_callback(&sensor_meas);
}

void loop(void) {
  /* nop */
}

// (0.040*AOP_AMP) = 0,944
// (0.200*AOP_AMP) = 4,720
// (4,720-0,944)   = 3,776
// 500 / 3,776 = 132.41525
// 50  / 3,776 = 13.241525
