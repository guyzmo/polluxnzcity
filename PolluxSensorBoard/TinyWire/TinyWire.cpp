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
#include <Arduino.h>
#include "TinyWire.h"

Buffer TinyWire::input_buffer;
Buffer TinyWire::output_buffer;
uint8_t TinyWire::last = 0xFF;
uint8_t TinyWire::type = 0xFF;
void (*TinyWire::_request_callback)(void);

void TinyWire::begin(uint8_t i2c_slave_address) {
    usiTwiSlaveInit(i2c_slave_address, 
                    _on_request_handler, 
                    _on_receive_handler);
    this->type = 0xFF;
}

int TinyWire::write(uint8_t data) {
    if (output_buffer.size() == BUFFER_SIZE)
        return -1;
    output_buffer.push(data);
    return output_buffer.size();
}

int TinyWire::write(char* str) {
    return output_buffer.push(str);
}
uint8_t TinyWire::available() {
    return input_buffer.size();
}
void TinyWire::set_type(uint8_t t) {
    type = t;
}
uint8_t TinyWire::read() {
    return input_buffer.pop();
}


void TinyWire::set_request_callback(void (*func)(void)) {
    _request_callback = func;
}

void TinyWire::_on_receive_handler(uint8_t reg, uint8_t value) {
    switch (reg) {
        default:
            input_buffer.push(reg);
            input_buffer.push(value);
    }
}

uint8_t TinyWire::_on_request_handler(uint8_t reg) {
    switch (reg) {
        case I2C_CMD_MEAS:
            output_buffer.clear();
            _request_callback();
            return 0;

        case I2C_TYPE:
            return type;
        case I2C_LEN:
            return output_buffer.size();
        case I2C_GET:
            last = I2C_GET;
        default:
            if (output_buffer.size() > 0)
                if (last == I2C_GET)
                    return output_buffer.pop();
                else
                    return 0x00;
            else
                last = 0xFF;
            return 0x00;
    }
}

