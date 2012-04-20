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

//////////////////////////////////////////////////////////////////////////////////////////
//// TinyWire.cpp


extern "C" {
    #include <usiTwiSlave.h>
}
#include "TinyWire.h"

void TinyWire::begin(uint8_t i2c_slave_address) {
    usiTwiSlaveInit(i2c_slave_address, 
                    _on_request_handler, 
                    _on_receive_handler);
    for (int i=0;i<I2C_ADDR_SIZE;++i)
        mem_buffer[i] = 0;
}

uint8_t TinyWire::read(uint8_t reg) {
    return mem_buffer[reg];
}
void TinyWire::write(uint8_t reg, uint8_t value) {
    mem_buffer[reg] = value;
}

void TinyWire::_on_receive_handler(uint8_t reg, uint8_t value) {
    mem_buffer[reg] = value;
}

uint8_t TinyWire::_on_request_handler(uint8_t reg) {
    return mem_buffer[reg];
}
