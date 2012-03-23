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

uint8_t gw_node[] = { 0x00, 0x13, 0xA2, 0x00, 0x40, 0x69, 0x86, 0x79 };

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


class XbeeResult {
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
                        printf("CMD %02X ", frame->content.rx.payload[0]);
                        command = frame->content.rx.payload[0];
                        printf("ADDR %02X ", frame->content.rx.payload[1]);
                        addr =  frame->content.rx.payload[1];
                        printf("REG %d ", frame->content.rx.payload[2]);
                        reg = frame->content.rx.payload[2];

                        // content
                        printf("LEN %d ", frame->content.rx.payload[3]);
                        length = frame->content.rx.payload[3];
                        printf("TYPE %02X ", frame->content.rx.payload[4]);
                        type = frame->content.rx.payload[4];
                        for (int i=0;i<length;++i) 
                            v.buffer[i] = (char)(frame->content.rx.payload)[i+5];
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
        int get_i2c_register() {
            return reg;
        }
        const char* fmt_i2c_command() {
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
};
//#endif __XBEERESULT_H__
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>


// [{"v":22.0,"u":"ppm","p":"50","k":"NO2"},
//  {"v":22.0,"u":"ppm","p":"50","k":"SO2"},
//  {"v":18.0,"u":"ppm","p":"50","k":"CO2"},
//  {"v":22.0,"u":"ppm","p":"50","k":"PM10"},
//  {"v":22.0,"u":"ppm","p":"50","k":"lat"},
//  {"v":22.0,"u":"ppm","p":"50","k":"lon"}]
const char* JSON_DATA_FMT  = "[%s]";
const char* JSON_VALUE_FMT = "{\"k\":%s, \"v\":%s, \"u\":%s, \"p\":%d}";

class Sensor {
    std::string name;
    std::string unit;

    uint8_t address;
    uint8_t reg;

    protected:
        bool ignored;

    public:
        Sensor(std::string name, std::string unit, uint8_t addr, uint8_t reg) : name(name), unit(unit), address(addr), reg(reg) {
            this->ignored = false;
        }
        std::string get_name() {
            return this->name;
        }
        std::string get_unit() {
            return this->unit;
        }
        uint8_t get_address() {
            return this->address;
        }
        uint8_t get_reg() {
            return this->reg;
        }
        virtual bool is_ignored() {
            return this->ignored;
        }
};
class Action : public Sensor {
    public:
        Action(std::string name, std::string unit, uint8_t addr, uint8_t reg) : Sensor(name,unit,addr,reg) {
            ignored = true;
        }
        bool is_ignored() {
            return ignored;
        }
};

class XbeePollux : public XbeeCommunicator {
    std::unordered_map<uint8_t, std::vector<Sensor>*> sensors_map;
    std::vector<std::string> values_json_list;

    int meas_idx;
    
    void setup_signal() {
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = sigint_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
    }
    public:
        XbeePollux(char* port, std::unordered_map<uint8_t, std::vector<Sensor>*> &sensors) : XbeeCommunicator(port), sensors_map(sensors) { 
            Beagle::UART::enable_uart2();
            Beagle::Leds::enable_leds();
            Beagle::Leds::set_status_led();
            this->setup_signal();
            meas_idx=0;
        }
        ~XbeePollux() {
            Beagle::Leds::disable_leds();
        }

        void store_measure(XbeeResult& payload) {
            std::ostringstream json_string;

            if (sensors_map.count(payload.get_i2c_address()) == 0) {
                printf("i2c address %02X is unknown.", payload.get_i2c_address());
                return;
            }
            if (sensors_map[payload.get_i2c_address()]->size() <= payload.get_i2c_register()) {
                printf("measure #%d at i2c address %02X is invalid.", payload.get_i2c_register(), payload.get_i2c_address());
                return;
            }
            if (sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).is_ignored()) {
                printf("measure from i2c(%02X,%02X) of type %d is ignored\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
                return;
            }

            switch (payload.get_type()) {
                case I2C_CHR:
                    json_string<<"{\"k\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_name()\
                               <<",\"v\":\""<<payload.get_value_as_char()\
                               <<",\"u\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_unit()\
                               <<",\"p\":\"0\"}";
                    break;
                case I2C_INT:
                    json_string<<"{\"k\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_name()\
                               <<",\"v\":\""<<payload.get_value_as_int()\
                               <<",\"u\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_unit()\
                               <<",\"p\":\"0\"}";
                    break;
                case I2C_FLT:
                    json_string<<"{\"k\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_name()\
                               <<",\"v\":\""<<payload.get_value_as_float()\
                               <<",\"u\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_unit()\
                               <<",\"p\":\"0\"}";
                    break;
                case I2C_DBL:
                    json_string<<"{\"k\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_name()\
                               <<",\"v\":\""<<payload.get_value_as_double()\
                               <<",\"u\":\""<<sensors_map[payload.get_i2c_address()]->at(payload.get_i2c_register()).get_unit()\
                               <<",\"p\":\"0\"}";
                    break;
                default:
                    printf("measure from i2c(%02X,%02X) of type %d unsupported\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
                    return;
            }
            printf("measure from i2c(%02X,%02X) : %s\n", payload.get_i2c_address(), payload.get_i2c_register(), json_string.str().c_str());
            values_json_list.push_back(json_string.str());
            return;
        }
        void push_data() {
            std::ostringstream json_string;

            json_string<<"[";
            for (std::vector<std::string>::iterator it=values_json_list.begin() ; it < values_json_list.end(); ++it )
                json_string << *it <<",";
            json_string<<"]";

            printf("Pushing data to citypulse: '%s'\n", json_string.str().c_str());
            
        }
        void get_next_measure() {
            printf("next measure: %d\n", meas_idx);
            switch (meas_idx++) {
                case 0:
                    printf("Getting Fan\n");
                    {
                        // XXX ask for the 3 measures of sensor 0x27
                        char buf[] = {CMD_MEAS, 0x27, 3};
                        send((char*)buf, gw_node, 0xFFFF);
                    }
                    break;
                case 1:
                    printf("Getting SPL\n");
                    break;
                case 2:
                    printf("Getting Temp\n");
                    break;
                case 3:
                    {
                        printf("Getting DUST\n");

                        // XXX ask for the 1 measure of sensor 0x26
                        char buf[] = {CMD_MEAS, 0x26, 1};
                        send((char*)buf, gw_node, 0xFFFF);
                    }
                    break;
                default:
                    {
                        meas_idx = 0;
                        printf("Sending HALT to device\n");

                        char buf[] = {CMD_HALT};
                        send((char*)buf, gw_node, 0xFFFF);
                        push_data();
                    }
            }
        }

        void run (XBeeFrame* frame) {

            //XbeeCommunicator::run(frame); // print frame details
            XbeeResult payload(frame);

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
                        case CMD_MEAS:
                            printf("GOT MEASURE\n");
                            store_measure(payload);
                            get_next_measure();                           
                            break;
                        case CMD_INIT:
                            printf("GOT WAKE UP\n");
                            get_next_measure();                           
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
    std::unordered_map<uint8_t, std::vector<Sensor>*> sensors_map;
    std::vector<Sensor>* sensor;

    sensor = new std::vector<Sensor>();
    sensor->push_back(Sensor("Dust","ppm",0x26, 0));
    sensors_map[0x26] = sensor;

    sensor = new std::vector<Sensor>();
    sensor->push_back(Action("Fan","",0x27, 0));
    sensor->push_back(Sensor("Noise level","dB",0x27, 1));
    sensor->push_back(Sensor("Temperature","ºC", 0x27, 2));
    sensors_map[0x27] = sensor;

    //sensor = new std::vector<Sensor>();
    //sensor->push_back(Sensor("CO","ppm",0x28));
    //sensors_map[0x28] = sensor;
    
    //sensor = new std::vector<Sensor>();
    //sensor->push_back(Sensor("NO2","ppm",0x29));
    //sensors_map[0x29] = sensor;

    XbeePollux s = XbeePollux("/dev/ttyO2",sensors_map);

    if (s.begin(panid,venid) >= 0) {
        for (;;) 
            s.poll();
        return 0;
    }
    return 1;
}

////// OLD CODE
                        /*
            Tokenizer* init_str;
            std::vector<std::string>* sensors;
                        case CMD_INIT:
                            // store 'payload' for address 'addr' into hashmap<int> _descriptors;
                            init_str = new Tokenizer(payload.get_value_as_string(), ";");
                            sensors = new std::vector<std::string>();

                            while (init_str->NextToken()) {
                                sensors->push_back(init_str->GetToken());
                            }
                            sensors_map[payload.get_i2c_address()] = sensors;
                            delete(init_str);
                            break;
                         */

