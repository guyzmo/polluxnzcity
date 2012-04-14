/*
 * Pollux'NZ City source code
 *
 * (c) 2012 CKAB / hackable:Devices
 * (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __XBEERESULT_H__
#define __XBEERESULT_H__

#include <xbee/xbee_constants.h>
#include <xbee/xbee_types.h>
#include <xbee/toolbox.h>

#include <string.h>
#include <stdio.h>

#define CMD_INIT 1
#define CMD_MEAS 8
#define CMD_HALT 32

#define I2C_LEN  2
#define I2C_TYPE 4
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16

class Xbee_result {
    // CONTENT
    int type;
    int length;

    // I2C
    int command;
    int addr;
    int reg;

    // ZIGBEE
    uint8_t node[8];
    int network;
    union multi_val {
        char buffer[33];
        double d;
        float f;
        int i;
        char c;
    } v;

    public:
        void reset();
        Xbee_result();
        Xbee_result(XBeeFrame* frame);
        // ZIGBEE
        char* fmt_node_address(char* str);
        uint8_t* get_node_address();
        unsigned long long get_node_address_as_long();
        int get_network();
        // I2C
        int get_i2c_address();
        int get_i2c_register();
        const char* fmt_i2c_command();
        int get_i2c_command();
        // CONTENT
        int get_type();
        const char* fmt_type();
        int get_length();
        double get_value_as_double();
        float get_value_as_float();
        int get_value_as_int();
        uint8_t get_value_as_uint8();
        char get_value_as_char();
        char* get_value_as_string();
        void print();
};
#endif // __XBEERESULT_H__
