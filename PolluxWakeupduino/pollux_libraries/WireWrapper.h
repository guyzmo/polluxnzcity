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
 */
#ifndef __WIREWRAPPER_H__
#define __WIREWRAPPER_H__

//#define VERBOSE
//#define NO_ZIGBEE

//////////////////////////////////////////////////////// I2C Library
#include <LedWrapper.h>
#include <Wire.h>

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

#define CMD_INIT 1
#define I2C_LEN  2
#define I2C_TYPE 4
#define CMD_MEAS 8
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16

uint8_t send_cmd(uint8_t addr, uint8_t cmd) {
    int err;

    dbg_printf("*** send_cmd: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("*** send_cmd: writing %d on %02X", cmd, addr);
    Wire.write(cmd);
    delay(TIMING);

    dbg_printf("*** send_cmd: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        //0 .. success
        //1 .. length to long for buffer
        //2 .. address send, NACK received
        //3 .. data send, NACK received
        //4 .. other twi error (lost bus arbitration, bus error, ..)
        
        dbg_printf("*** error #%d in i2c write.", err);
        flashLed(LED_ERR, 1, 10);
        delay(TIMING);
        return err;
    } else { 
        flashLed(LED_STA, 1, 10);
        return 0;
    }
    
}

uint8_t send_cmd(uint8_t addr, uint8_t cmd, uint8_t val) {
    int err;

    dbg_printf("*** send_cmd2: trying device %02X", addr);

    Wire.beginTransmission(addr);
    delay(TIMING);

    dbg_printf("*** send_cmd2: writing %02X for %02X on %02X", val, cmd, addr);
    Wire.write(cmd);
    Wire.write(val);
    delay(TIMING);

    dbg_printf("*** send_cmd2: detecting i2c tx error");

    if ((err = Wire.endTransmission()) != 0) {
        dbg_printf("***  error #%d in i2c write2.", err);
        delay(TIMING);
        return err;
    } else {
        flashLed(LED_STA, 1, 10);
        return 0;
    }
    
}

uint8_t recv(int addr, int n, char* buf) {
    int j=0;

    dbg_printf("*** recv: trying to get %d byte%s.", n, (n<1?"":"s"));

    Wire.requestFrom(addr, n);
    delay(TIMING);

    while (Wire.available() && j<=n) {
        delay(TIMING);
        buf[j] = Wire.read();
        delay(TIMING);
        dbg_printf("*** recv: rcving byte #%d: %02X", j, buf[j]);
        ++j;
    }
    if (j == 0) {
        dbg_printf("*** recv: error rcving bytes.");
        flashLed(LED_ERR, 1, 10);
    } else {
        buf[j] = '\0';
        dbg_printf("*** recv: received %s", buf);
        flashLed(LED_STA, 1, 10);
    }
    return j;
}

typedef struct i2c_result {
    int command;
    int type;
    int len;
    union multi_val {
        char buffer[32];
        double d;
        float f;
        int i;
        char c;
    } v;
    void reset() { for (int i=0;i<32;++i) v.buffer[i] = 0; }
    i2c_result() { reset(); }
} i2c_result;

//////
#endif // __WIREWRAPPER_H__
