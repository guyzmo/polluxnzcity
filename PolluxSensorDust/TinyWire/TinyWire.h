#ifndef __TINYWIRE_H__
#define __TINYWIRE_H__

/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 * 
 * Copyright (c) 2009 Andrew Rapp. For the XBee example origins.
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
extern "C" {
#include "usiTwiSlave.h"
}
#include "buffer.h"

#define I2C_CMD_INIT 1
#define I2C_CMD_MEAS 8

#define I2C_LEN  2
#define I2C_TYPE 4
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16

class TinyWire {
    static Buffer input_buffer;
    static Buffer output_buffer;
    static uint8_t last;

    static uint8_t type;

    static void (*_request_callback)(void);

    static void _on_receive_handler(uint8_t reg, uint8_t value);
    static uint8_t _on_request_handler(uint8_t reg);

    public:
        void begin(uint8_t i2c_slave_address);
        void set_type(uint8_t t);

        int write(uint8_t data);
        int write(char* str);

        uint8_t available();
        uint8_t read();

        static void set_request_callback(void (*func)(void));
};

#endif // __TINYWIRE_H__
