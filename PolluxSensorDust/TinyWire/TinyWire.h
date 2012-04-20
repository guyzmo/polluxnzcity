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
#define I2C_ADDR_SIZE 256

class TinyWire {
    static uint8_t mem_buffer[I2C_ADDR_SIZE];

    static void _on_receive_handler(uint8_t reg, uint8_t value);
    static uint8_t _on_request_handler(uint8_t reg);

    public:
        void begin(uint8_t i2c_slave_address);

        uint8_t read(uint8_t reg);
        void write(uint8_t reg, uint8_t value);

};

#endif // __TINYWIRE_H__
