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

#include <xbee/xbee_result.h>

using namespace xbee;

void Xbee_result::reset() { 
    for (int i=0;i<32;++i) v.buffer[i] = 0; 
    for (int i=0;i<8;++i) node[i] = 0; 
    type = -1;
    length = 0;
    command = -1;
    addr = -1;
    network = -1;
}

Xbee_result::Xbee_result() { 
    reset(); 
}

Xbee_result::Xbee_result(XBeeFrame* frame) { 
    reset();
    switch (frame->api_id) {
        case RX_PACKET:
            // zigbee
            for (int i=0;i<8;++i) 
                node[i] = frame->content.rx.source_addr[i]; 

            network = (int)frame->content.rx.network_addr.i16;

            // if frame is a comment
            if (frame->content.rx.payload[0] == 42) {

                length = strlen((char*)frame->content.rx.payload);
                if (length > 32) length = 32;

                type = 42;
                command = 42;

                for (int i=0;i<length;++i) 
                    v.buffer[i] = ((char*)frame->content.rx.payload)[i];
                v.buffer[32] = '\0';

            } else {
                // i2c
                debug_printf("Parsing results: CMD %02X ", frame->content.rx.payload[0]);
                command = frame->content.rx.payload[0];
                debug_printf("ADDR %02X ", frame->content.rx.payload[1]);
                addr =  frame->content.rx.payload[1];
                debug_printf("REG %d ", frame->content.rx.payload[2]);
                reg = frame->content.rx.payload[2];

                // content
                debug_printf("LEN %d ", frame->content.rx.payload[3]);
                length = frame->content.rx.payload[3];
                debug_printf("TYPE %02X\n", frame->content.rx.payload[4]);
                type = frame->content.rx.payload[4];
                for (int i=0;i<length;++i) 
                    v.buffer[i] = (char)(frame->content.rx.payload)[i+5];
            }
            break;
        case AT_CMD_RESP:
            for (unsigned int i=0;i<strlen((char*)frame->content.rx.payload+4);++i) 
                v.buffer[i] = ((char*)frame->content.rx.payload+4)[i];
            break;
    }
}


// ZIGBEE
char* Xbee_result::fmt_node_address(char* str) {
    sprintf(str, "%02X%02X%02X%02X:%02X%02X%02X%02X", node[0],node[1],node[2],node[3],node[4],node[5],node[6],node[7] );
    return str;
}

uint8_t* Xbee_result::get_node_address() {
    return node;
}

unsigned long long Xbee_result::get_node_address_as_long() {
    uint64_t addr = 0;
    for (int i=0;i<8;++i) {
        addr = (addr<<8) | node[i];
    }

    debug_printf("get_node_address_as_long(): long addr: %llx\n", addr);

    return addr;
}

int Xbee_result::get_network() {
    return network;
}


// I2C
int Xbee_result::get_i2c_address() {
    return addr;
}

int Xbee_result::get_i2c_register() {
    return reg;
}

const char* Xbee_result::fmt_i2c_command() {
    switch (command) {
        case CMD_INIT:
            return "CMD_INIT";
        case CMD_MEAS:
            return "CMD_MEAS";
        case 42:
            return "COMMENT";
        default:
            return "UNKNOWN CMD";
    }
}

int Xbee_result::get_i2c_command() {
    return command;
}


// CONTENT
int Xbee_result::get_type() {
    return type;
}

const char* Xbee_result::fmt_type() {
    switch (type) {
        case I2C_CHR:
            return "char";
        case I2C_INT:
            return "int";
        case I2C_FLT:
            return "flt";
        case I2C_DBL:
            return "dbl";
        case I2C_STR:
            return "str";
        case 42:
            return "comment";
        default:
            return "ERR";
    }
}

int Xbee_result::get_length() {
    return length;
}

double Xbee_result::get_value_as_double() {
    return v.d;
}

float Xbee_result::get_value_as_float() {
    return v.f;
}

int Xbee_result::get_value_as_int() {
    return v.i;
}

uint8_t Xbee_result::get_value_as_uint8() {
    return (uint8_t)v.c;
}

char Xbee_result::get_value_as_char() {
    return v.c;
}

char* Xbee_result::get_value_as_string() {
    return v.buffer;
}

void Xbee_result::print() {
    char netstr[8];
    if (command == 42) {
        debug_printf("[RX] zigbee(Net:'%02X',Src:'%s') ; Content(len:%d,type:%s) : '%s'\n", get_network(), 
                fmt_node_address((char*)netstr), 
                strlen(get_value_as_string()),
                fmt_type(),
                get_value_as_string());
    } else
        switch (type) {
            case I2C_INT:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X:%d) ; Content(len:%d,type:%s) : '%d'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        fmt_i2c_command(),
                        get_i2c_address(), 
                        get_i2c_register(), 
                        get_length(),
                        fmt_type(),
                        get_value_as_int());
                break;
            case I2C_FLT:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X:%d) ; Content(len:%d,type:%s) : '%5.2f'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        fmt_i2c_command(),
                        get_i2c_address(), 
                        get_i2c_register(), 
                        get_length(),
                        fmt_type(),
                        get_value_as_float());
                break;
            case I2C_DBL:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X:%d) ; Content(len:%d,type:%s) : '%5.2f'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        fmt_i2c_command(),
                        get_i2c_address(), 
                        get_i2c_register(), 
                        get_length(),
                        fmt_type(),
                        get_value_as_double());
                break;
            case I2C_CHR:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X:%d) ; Content(len:%d,type:%s) : '%c'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        fmt_i2c_command(),
                        get_i2c_address(), 
                        get_i2c_register(), 
                        get_length(),
                        fmt_type(),
                        get_value_as_char());
                break;
            case I2C_STR:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X:%d) ; Content(len:%d,type:%s) : '%s'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        fmt_i2c_command(),
                        get_i2c_address(), 
                        get_i2c_register(), 
                        get_length(),
                        fmt_type(),
                        get_value_as_string());
                break;
            case 42:
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; Content(len:%d,type:%s) : '%s'\n", get_network(), 
                        fmt_node_address((char*)netstr), 
                        get_length(),
                        fmt_type(),
                        get_value_as_string());
                break;

        }
}
