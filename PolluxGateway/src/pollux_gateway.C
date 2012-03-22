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
 * Main routines
 */

//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include <string>

//#include <citypulse.h>
#include <beaglebone.h>
#include <xbee_communicator.h>

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

void sigint_handler(int c) {
    Beagle::Leds::disable_leds();
    Beagle::UART::disable_uart2();
    printf("Exiting...\n");
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
//#ifndef __TOKENIZER_H__
//#define __TOKENIZER_H__
//#include <string>
class Tokenizer {
    protected:
        size_t m_offset;
        const std::string m_string;
        std::string m_token;
        std::string m_delimiters;
    public:
        Tokenizer(const std::string& s) :
            m_string(s), 
            m_offset(0), 
            m_delimiters(" \t\n\r") {}

        Tokenizer(const std::string& s, const std::string& delimiters) :
            m_string(s), 
            m_offset(0), 
            m_delimiters(delimiters) {}

        bool NextToken() {
            return NextToken(m_delimiters);
        }

        bool NextToken(const std::string& delimiters) {
            size_t i = m_string.find_first_not_of(delimiters, m_offset);
            if (std::string::npos == i) 
            {
                m_offset = m_string.length();
                return false;
            }

            size_t j = m_string.find_first_of(delimiters, i);
            if (std::string::npos == j) 
            {
                m_token = m_string.substr(i);
                m_offset = m_string.length();
                return true;
            }

            m_token = m_string.substr(i, j - i);
            m_offset = j;
            return true;
        }
        const std::string GetToken() const {
            return m_token;
        }
        void Reset() {
        }
};
//#endif //__TOKENIZER_H__
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//#ifndef __XBEERESULT_H__
//#define __XBEERESULT_H__

#define I2C_CMD_INIT 1
#define I2C_LEN  2
#define I2C_TYPE 4
#define I2C_CMD_MEAS 8
#define I2C_GET 16

#define I2C_INT 1
#define I2C_FLT 2
#define I2C_DBL 4
#define I2C_CHR 8
#define I2C_STR 16


class XbeeResult {
    // CONTENT
    int type;
    int length;
    // I2C
    int command;
    int addr;
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
        void reset() { 
            for (int i=0;i<32;++i) v.buffer[i] = 0; 
            for (int i=0;i<8;++i) node[i] = 0; 
            type = -1;
            length = 0;
            command = -1;
            addr = -1;
            network = -1;
        }
        XbeeResult() { 
            reset(); 
        }
        XbeeResult(XBeeFrame* frame) { 
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
                        command = frame->content.rx.payload[0];
                        addr =  frame->content.rx.payload[1];

                        // content
                        length = frame->content.rx.payload[2];
                        type = frame->content.rx.payload[3];
                        for (int i=0;i<length;++i) 
                            v.buffer[i] = (char)(frame->content.rx.payload)[i+4];
                    }
                    break;
                case AT_CMD_RESP:
                    for (int i=0;i<strlen((char*)frame->content.rx.payload+4);++i) 
                        v.buffer[i] = ((char*)frame->content.rx.payload+4)[i];
                    break;
            }
        }
        // ZIGBEE
        char* fmt_node_address(char* str) {
            sprintf(str, "%02X%02X%02X%02X:%02X%02X%02X%02X", node[0],node[1],node[2],node[3],node[4],node[5],node[6],node[7] );
            return str;
        }
        uint8_t* get_node_address() {
            return node;
        }
        int get_network() {
            return network;
        }
        // I2C
        int get_i2c_address() {
            return addr;
        }
        const char* fmt_i2c_command() {
            switch (command) {
                case I2C_CMD_INIT:
                    return "I2C_CMD_INIT";
                case I2C_CMD_MEAS:
                    return "I2C_CMD_MEAS";
                case 42:
                    return "COMMENT";
                default:
                    return "UNKNOWN CMD";
            }
        }
        int get_i2c_command() {
            return command;
        }
        // CONTENT
        int get_type() {
            return type;
        }
        const char* fmt_type() {
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
            }
        }
        int get_length() {
            return length;
        }
        double get_value_as_double() {
            return v.d;
        }
        float get_value_as_float() {
            return v.f;
        }
        int get_value_as_int() {
            return v.i;
        }
        uint8_t get_value_as_uint8() {
            return (uint8_t)v.c;
        }
        char get_value_as_char() {
            return v.c;
        }
        char* get_value_as_string() {
            return v.buffer;
        }
        void print() {
            char netstr[8];
            if (command == 42)
                printf("[RX] zigbee(Net:'%02X',Src:'%s') ; Content(len:%d,type:%s) : '%s'\n", get_network(), 
                                                                                                fmt_node_address((char*)netstr), 
                                                                                                strlen(get_value_as_string()),
                                                                                                fmt_type(),
                                                                                                get_value_as_string());
            else
                switch (type) {
                    case I2C_INT:
                        printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X) ; Content(len:%d,type:%s) : '%d'\n", get_network(), 
                                                                                                                                fmt_node_address((char*)netstr), 
                                                                                                                                fmt_i2c_command(),
                                                                                                                                get_i2c_address(), 
                                                                                                                                get_length(),
                                                                                                                                fmt_type(),
                                                                                                                                get_value_as_int());
                        break;
                    case I2C_FLT:
                        printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X) ; Content(len:%d,type:%s) : '%5.2f'\n", get_network(), 
                                                                                                                                    fmt_node_address((char*)netstr), 
                                                                                                                                    fmt_i2c_command(),
                                                                                                                                    get_i2c_address(), 
                                                                                                                                    get_length(),
                                                                                                                                    fmt_type(),
                                                                                                                                    get_value_as_float());
                        break;
                    case I2C_DBL:
                        printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X) ; Content(len:%d,type:%s) : '%5.2f'\n", get_network(), 
                                                                                                                                    fmt_node_address((char*)netstr), 
                                                                                                                                    fmt_i2c_command(),
                                                                                                                                    get_i2c_address(), 
                                                                                                                                    get_length(),
                                                                                                                                    fmt_type(),
                                                                                                                                    get_value_as_double());
                        break;
                    case I2C_CHR:
                        printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X) ; Content(len:%d,type:%s) : '%c'\n", get_network(), 
                                                                                                                                fmt_node_address((char*)netstr), 
                                                                                                                                fmt_i2c_command(),
                                                                                                                                get_i2c_address(), 
                                                                                                                                get_length(),
                                                                                                                                fmt_type(),
                                                                                                                                get_value_as_char());
                        break;
                    case I2C_STR:
                        printf("[RX] zigbee(Net:'%02X',Src:'%s') ; i2c('%s',sensor:%02X) ; Content(len:%d,type:%s) : '%s'\n", get_network(), 
                                                                                                                                fmt_node_address((char*)netstr), 
                                                                                                                                fmt_i2c_command(),
                                                                                                                                get_i2c_address(), 
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
};
//#endif __XBEERESULT_H__
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <unordered_map>


// [{"v":22.0,"u":"ppm","p":"50","k":"NO2"},
//  {"v":22.0,"u":"ppm","p":"50","k":"SO2"},
//  {"v":18.0,"u":"ppm","p":"50","k":"CO2"},
//  {"v":22.0,"u":"ppm","p":"50","k":"PM10"},
//  {"v":22.0,"u":"ppm","p":"50","k":"lat"},
//  {"v":22.0,"u":"ppm","p":"50","k":"lon"}]
const char* JSON_DATA_FMT  = "[%s]";
const char* JSON_VALUE_FMT = "{\"k\":%s, \"v\":%s, \"u\":%s}";

class XbeePollux : public XbeeCommunicator {
    std::unordered_map<int, std::vector<std::string>*> sensors_map;
    
    void setup_signal() {
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = sigint_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
    }
    public:
        XbeePollux(char* port) : XbeeCommunicator(port) { 
            Beagle::UART::enable_uart2();
            Beagle::Leds::enable_leds();
            Beagle::Leds::set_status_led();
            this->setup_signal();
        }
        ~XbeePollux() {
            Beagle::Leds::disable_leds();
        }

        void run (XBeeFrame* frame) {

            XbeeCommunicator::run(frame); // print frame details
            XbeeResult payload(frame);
            Tokenizer* init_str;
            std::vector<std::string>* sensors;

            switch (frame->api_id) {
                case AT_CMD_RESP:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::GREEN);

                    printf("[AT] Command: '%s' ; Values: '%X'\n", frame->content.at.command, frame->content.at.values);

                    msleep(50);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::GREEN);
                    break;
                case RX_PACKET:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::BLUE);

                    payload.print();

                    switch (payload.get_i2c_command()) {
                        case I2C_CMD_INIT:
                            // store 'payload' for address 'addr' into hashmap<int> _descriptors;
                            init_str = new Tokenizer(payload.get_value_as_string(), ";");
                            sensors = new std::vector<std::string>();

                            while (init_str->NextToken()) {
                                sensors->push_back(init_str->GetToken());
                            }
                            sensors_map[payload.get_i2c_address()] = sensors;
                            delete(init_str);
                            break;
                        case I2C_CMD_MEAS:
                            printf("MEAS\n");
                            {
                                msleep(2000);
                                uint8_t gw_node[] = { 0x00, 0x13, 0xA2, 0x00, 0x40, 0x69, 0x86, 0x79 };
                                send((char*)"TEST", gw_node, 0xFFFF);
                                printf("SENT\n");
                            }

                            break;
                        case '*':
                            // just a comment ;)
                            break;
                        default:
                            printf("Unknown Command: %d\n", payload.fmt_i2c_command());
                    }
                    /* XXX 1. constructs the JSON with data from payload */
                    /* XXX 2. call post_to_citypulse(json_str); */

                    msleep(10);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::BLUE);
                    break;
                default:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::RED);
                    msleep(10);
                    printf("Incoming frame (%02X) that's not useful.\n", frame->api_id);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::RED);
                    break;
            }
        }
};

int main(void) {
    XbeePollux s = XbeePollux("/dev/ttyO2");

    if (s.begin(panid,venid) >= 0) {
        for (;;) 
            s.poll();
        return 0;
    }
    return 1;
}


