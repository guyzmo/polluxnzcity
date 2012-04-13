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
#include <iostream>

//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include <string>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <pollux_toolbox.h>
#include <citypulse.h>
#include <beaglebone.h>
#include <xbee_communicator.h>

const int panid[2] = {0x2,0xA};
const int venid[8] = {0x0, 0x0, 0x1, 0x3, 0xA, 0x2, 0x0, 0x0};

const uint8_t gw_node[] = { 0x00, 0x13, 0xA2, 0x00, 0x40, 0x69, 0x86, 0x75 };
const uint64_t gw_node_l = 0x0013A20040698675;

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
        void reset() { 
            for (int i=0;i<32;++i) v.buffer[i] = 0; 
            for (int i=0;i<8;++i) node[i] = 0; 
            type = -1;
            length = 0;
            command = -1;
            addr = -1;
            network = -1;
        }
        Xbee_result() { 
            reset(); 
        }
        Xbee_result(XBeeFrame* frame) { 
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
                        printf("TYPE %02X\n", frame->content.rx.payload[4]);
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
        unsigned long long get_node_address_as_long() {
            uint64_t addr = 0;
            for (int i=0;i<8;++i) {
                addr = (addr<<8) | node[i];
            }

            debug_printf("get_node_address_as_long(): long addr: %llx\n", addr);

            return addr;
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
};
//#endif __XBEERESULT_H__
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//#ifndef __POLLUX_CONFIGURATOR_H__
//#define __POLLUX_CONFIGURATOR_H__
//#include <stdio.h>
//#include <stdlib.h>
//#include <stddef.h>

#include <json/json.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include <stdexcept>

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
        Action(std::string name, uint8_t addr, uint8_t reg) : Sensor(name,"",addr,reg) {
            ignored = true;
        }
        bool is_ignored() {
            return ignored;
        }
};

typedef std::unordered_map<unsigned long long /* ZigBee Address */, std::unordered_map<uint8_t /* i2c addr */, std::vector<Sensor>/* sensor list */ > > long_short_sensor_map;


inline bool json_object_has_key(struct json_object* obj, const std::string& key) {
    json_object_object_foreach(obj, k,v) {
        if (std::string(k) == key) 
            return true; 
    }
    return false;
}

class Pollux_config_exception : public std::runtime_error {
    public:
        Pollux_config_exception(const std::string& m) : runtime_error(m) {}
};
class Pollux_configurator {
    mutable long_short_sensor_map sensors_map;
    mutable string_string_string_map datastores_map;
    mutable string_string_map configuration_map;
    mutable string_string_map geoloc_map;
       
    std::vector<string_string_map*> values_list;

    typedef struct {
        unsigned int meas_idx;
        unsigned int stop;
        std::unordered_map<uint8_t, std::vector<Sensor> >::iterator it;
    } module_iter;
    std::unordered_map<unsigned long long, module_iter> module_iterator_map;

    std::string& path;

    public:
        Pollux_configurator(std::string& path) : path(path){
        }
        const std::string& get_config_option(std::string key) const {
            return configuration_map[key];
        }
        const std::string& get_datastore_value(std::string& ds, std::string& value) const {
            return datastores_map[ds][value];
        }

        void load_configuration() {
            std::ostringstream fname;
            std::cout<<path<<std::endl;

            fname << path << "config.json";
            struct json_object *json_data;

            json_data = json_object_from_file((char*)fname.str().c_str());
            if (is_error(json_data)) {
                throw Pollux_config_exception("Invalid JSON file. Please check your configuration file.");
            }
            if (!json_object_has_key(json_data,"configuration")) {
                throw Pollux_config_exception("Missing 'configuration' token in configuration file.");
            }

            struct json_object* config = json_object_object_get(json_data, "configuration");

            if (!json_object_has_key(config,"tty_port")) {
                throw Pollux_config_exception("Missing 'tty_port' token in configuration section.");
            }
            if (!json_object_has_key(config,"wud_sleep_time")) {
                throw Pollux_config_exception("Missing 'wud_sleep_time' token in configuration section.");
            }

            json_object_object_foreach(config,key,value) {
                configuration_map[key] = json_object_get_string(json_object_object_get(config,key));
#ifdef VERBOSE
                std::cout<<"config."<<key<<": "<<json_object_get_string(json_object_object_get(config,key))<<std::endl;
#endif //VERBOSE
            }

            free(json_data);
        }
        void load_geoloc() {
            std::ostringstream fname;
            std::cout<<path<<std::endl;

            fname << path << "config.json";
            struct json_object *json_data;

            json_data = json_object_from_file((char*)fname.str().c_str());
            if (is_error(json_data)) {
                throw Pollux_config_exception("Invalid JSON file. Please check your configuration file.");
            }
            if (!json_object_has_key(json_data,"geolocalisation")) {
                throw Pollux_config_exception("Missing 'geolocalisation' token in configuration file.");
            }

            struct json_object* config = json_object_object_get(json_data, "geolocalisation");

            if (!json_object_has_key(config,"latitude")) {
                throw Pollux_config_exception("Missing 'latitude' token in geolocalisation section.");
            }
            if (!json_object_has_key(config,"longitude")) {
                throw Pollux_config_exception("Missing 'longitude' token in geolocalisation section.");
            }

            json_object_object_foreach(config,key,value) {
                geoloc_map[key] = json_object_get_string(json_object_object_get(config,key));
#ifdef VERBOSE
                std::cout<<"geoloc."<<key<<": "<<json_object_get_string(json_object_object_get(config,key))<<std::endl;
#endif //VERBOSE
            }

            free(json_data);
        }
        void load_datastores() {
            struct json_object *json_data;
            std::ostringstream fname;

            fname << path << "config.json";

            json_data = json_object_from_file((char*)fname.str().c_str());
            if (is_error(json_data)) {
                throw Pollux_config_exception("Invalid JSON file. Please check your configuration file.");
            }
        
            if (!json_object_has_key(json_data,"datastores")) {
                throw Pollux_config_exception("Missing 'datastores' token in configuration file.");
            }

            struct json_object* datastores = json_object_object_get(json_data, "datastores");

            json_object_object_foreach(datastores,name,datastore) {
                json_object_object_foreach(datastore,key,value) {
#ifdef VERBOSE
                    printf("datastore(%s).%s: '%s'\n", name, key, json_object_get_string(json_object_object_get(datastore,key)));
#endif //VERBOSE
                    datastores_map[name][key] = json_object_get_string(value);
                }
            }

            free(json_data);
            return;
        }
        void load_sensors() {
            struct json_object *json_data;
            std::ostringstream fname;

            fname << path << "sensors.json";

            json_data = json_object_from_file((char*)fname.str().c_str());

            if (is_error(json_data)) {
                throw Pollux_config_exception("Invalid JSON file. Please check your configuration file.");
            }
            
            json_object_object_foreach(json_data,module_name,module) {
                unsigned long long zb_address;
                std::istringstream* ss = new std::istringstream( module_name );
                (*ss) >> std::hex >> zb_address;
                delete(ss);

                // if sensor declaration has name, address and register declared
                for (int i=0;i<json_object_array_length(module);++i) {
                    struct json_object* sensor_module = json_object_array_get_idx(module, i);
                    // skip deactivated modules
                    if (json_object_has_key(sensor_module, "activated"))
                        if (! json_object_get_boolean(json_object_object_get(sensor_module,"activated")))
                            continue;

                    if (json_object_has_key(sensor_module, "name") && json_object_has_key(sensor_module, "address") && json_object_has_key(sensor_module, "register")) {
                        unsigned short int i2c_address, reg;
                        ss = new std::istringstream( json_object_get_string(json_object_object_get(sensor_module, "address"))+2 );
                        (*ss) >> std::hex >> i2c_address;
                        delete(ss);
                        // check if the address has correctly been casted to int
                        ss = new std::istringstream( json_object_get_string(json_object_object_get(sensor_module, "register")) );
                        (*ss) >> std::hex >> reg;
                        delete(ss);
                        // if it is a Sensor module (i.e. that has a 'unit' key)
                        if (json_object_has_key(sensor_module, "unit")) {
#ifdef VERBOSE
                            std::cout<<"module(0x"<<std::setw(16)<<std::setfill('0')<<std::hex<<zb_address<<":0x"<<i2c_address<<") := sensor("\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"name"))<<","\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"unit"))\
                                                                                                       <<",0x"<<std::hex<<i2c_address<<",0x"<<std::hex<<reg<<")"<<std::endl;
#endif
                            sensors_map[zb_address][i2c_address].push_back(Sensor(json_object_get_string(json_object_object_get(sensor_module,"name")),
                                        json_object_get_string(json_object_object_get(sensor_module,"unit")),
                                        i2c_address, reg));
                            // if it is an Action module (i.e. that has *no* 'unit' key)
                        } else  {
                            sensors_map[zb_address][i2c_address].push_back(Action(json_object_get_string(json_object_object_get(sensor_module,"name")),
                                        i2c_address, reg));
#ifdef VERBOSE
                            std::cout<<"module(0x"<<std::setw(16)<<std::setfill('0')<<std::hex<<zb_address<<":0x"<<i2c_address<<") := action("\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"name"))\
                                                                                                       <<",0x"<<std::hex<<i2c_address<<",0x"<<std::hex<<reg<<")"<<std::endl;
#endif
                        }
                    }
                    module_iterator_map[zb_address].it = sensors_map[zb_address].begin();
                    module_iterator_map[zb_address].meas_idx=-1;
                    module_iterator_map[zb_address].stop=0;
                }

                free(json_data);
            }
        }

        char* next_measure(unsigned long long int module) {
            std::unordered_map<uint8_t, std::vector<Sensor> >::iterator& sensor_it = module_iterator_map[module].it;
            char* buf = (char*)malloc(sizeof(char)*3);
            buf[0] = 0x0;
            buf[1] = 0x0;
            buf[2] = 0x0;
            
            if (sensors_map.size() == 0) {
                debug_print("sensors hashmap is empty. Can't get a measure.\n");
                return buf;

            }

            if (sensors_map.find(module) == sensors_map.end()) {
                debug_printf("can't find module 0x%llx in sensors_map. Unknown zigbee module !\n", module);
                return buf;
            }

            if (sensor_it == sensors_map[module].end()) {
                sensor_it = sensors_map[module].begin();
                buf[0] = CMD_HALT;
                buf[1] = 0x0;
                buf[2] = 0x0;
                printf("************** SENDING HALT COMMAND\n");
                return buf;
            } else if (sensor_it->second.size() > 1)
                if (module_iterator_map[module].stop == 0) {
                    module_iterator_map[module].stop = sensor_it->second.size()-1;
                    module_iterator_map[module].meas_idx = 0;
                } else if (module_iterator_map[module].meas_idx < module_iterator_map[module].stop) {
                    ++(module_iterator_map[module].meas_idx);
                    /*debug_*/printf("************** SKIPPING MEASURE\n");
                    return buf;
                } else if (module_iterator_map[module].meas_idx == module_iterator_map[module].stop) {
                    module_iterator_map[module].stop = 0;
                    module_iterator_map[module].meas_idx = 0;
                    /*debug_*/printf("************** LAST SKIPPING MEASURE\n");

                    ++sensor_it;

                    free(buf);
                    return next_measure(module);
                }

            buf[0] = CMD_MEAS;
            buf[1] = (unsigned short int)(*sensor_it).first;
            buf[2] = (unsigned short int)(*sensor_it).second.size();

            /*debug_*/printf("************** MEASURE(S) TO SEND: ");
            for (int i=0;i<(*sensor_it).second.size();++i)
                /*debug_*/printf("%s, ", (*sensor_it).second[i].get_name().c_str());
            /*debug_*/printf("\n");

            if (sensor_it->second.size() == 1) 
                ++sensor_it;

            return buf;
        }
        void store_measure(Xbee_result& payload) {
            string_string_map* values = new string_string_map();
            std::ostringstream strconv;

            if (sensors_map[gw_node_l].count(payload.get_i2c_address()) == 0) {
                printf("i2c address %02X is unknown.", payload.get_i2c_address());
                return;
            }
            if (sensors_map[gw_node_l][payload.get_i2c_address()].size() <= payload.get_i2c_register()) {
                printf("measure #%d at i2c address %02X is invalid.", payload.get_i2c_register(), payload.get_i2c_address());
                return;
            }
            if (sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).is_ignored()) {
                printf("measure from i2c(%02X,%02X) of type %d is ignored\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
                return;
            }

            switch (payload.get_type()) {
                case I2C_CHR:
                    strconv<<payload.get_value_as_char();
                    (*values)["k"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
                    (*values)["v"] = strconv.str();
                    (*values)["u"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
                    (*values)["p"] = "0";
                    break;
                case I2C_INT:
                    strconv<<payload.get_value_as_int();
                    (*values)["k"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
                    (*values)["v"] = strconv.str();
                    (*values)["u"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
                    (*values)["p"] = "0";
                    break;
                case I2C_FLT:
                    strconv<<payload.get_value_as_float();
                    (*values)["k"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
                    (*values)["v"] = strconv.str();
                    (*values)["u"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
                    (*values)["p"] = "0";
                    break;
                case I2C_DBL:
                    strconv<<payload.get_value_as_float();
                    (*values)["k"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
                    (*values)["v"] = strconv.str();
                    (*values)["u"] = sensors_map[gw_node_l][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
                    (*values)["p"] = "0";
                    break;
                default:
                    printf("measure from i2c(%02X,%02X) of type %d unsupported\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
                    return;
            }
            values_list.push_back(values);
            printf("measure from i2c(%02X,%02X)\n", payload.get_i2c_address(), payload.get_i2c_register());//, json_string.str().c_str());
            return;
        }
        
        void store_csv(std::vector<string_string_map*>& val_list) {
            ///// CSV PART
            std::ostringstream csv_string;

            /// date format
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [80];

            time ( &rawtime );
            timeinfo = localtime ( &rawtime );

            strftime (buffer,80,"%Y/%m/%d %H:%M:%S",timeinfo);
            csv_string << buffer <<",";
            /// date format
            
            for (std::vector<string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it) {
                if ((**val_it)["k"] == "longitude" or (**val_it)["k"] == "latitude" or (**val_it)["k"] == "altitude")
                    continue;
                csv_string<<",\"u\":\""<<(**val_it)["u"];
                if (val_it+1 != values_list.end())
                    csv_string<<",";
                csv_string<<std::endl;
            }

            FILE* fd = fopen("sensor_values.csv","a");
            fputs(csv_string.str().c_str(),fd);
            fclose(fd);
            std::cout<<"written data to file !"<<std::endl;
            ///// CSV PART
        }

        void push_data(long long unsigned int module) {
            std::ostringstream val_string;

            if (geoloc_map.find("latitude") != geoloc_map.end() and geoloc_map.find("longitude") != geoloc_map.end() /*and geoloc_map.find("altitude") != geoloc_map.end()*/) {
                string_string_map* values = new string_string_map();
                (*values)["k"] = "latitude";
                (*values)["v"] = geoloc_map["latitude"];
                values_list.push_back(values);

                values = new string_string_map();
                (*values)["k"] = "longitude";
                (*values)["v"] = geoloc_map["longitude"];
                values_list.push_back(values);

                /*
                values = new string_string_map();
                (*values)["k"] = "altitude";
                (*values)["v"] = geoloc_map["altitude"];
                values_list.push_back(values);
                */
            }

            for (string_string_string_map::iterator store_it = datastores_map.begin();store_it!=datastores_map.end();++store_it) {
                if (store_it->second["activated"] != "false") {
                    if (store_it->first == "citypulse") {
                        printf("citypulse\n");

                        if (citypulse_post(values_list, store_it->second) == 0) {
                            printf("    -> success\n");
                            Beagle::Leds::set_rgb_led(Beagle::Leds::GREEN);
                            msleep(100);
                            Beagle::Leds::reset_rgb_led(Beagle::Leds::GREEN);
                        } else {
                            printf("    -> failure\n");
                            Beagle::Leds::set_rgb_led(Beagle::Leds::RED);
                            msleep(100);
                            Beagle::Leds::reset_rgb_led(Beagle::Leds::RED);
                        }
                    } else if  (store_it->first == "pachube") {
                        printf("pachube\n");
                    } else if  (store_it->first == "local") {
                        printf("local\n");
                        
                        store_csv(values_list); 
                    }
                    for (std::vector<string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it)
                        delete(*val_it);
                    values_list.clear();
                }
            }

        }
};

//#endif // __POLLUX_CONFIGURATOR_H__
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// #ifndef __XBEE_POLLUX_H__
// #define __XBEE_POLLUX_H__

void sigint_handler(int c) {
    Beagle::Leds::disable_leds();
    Beagle::UART::disable_uart2();
    printf("Exiting...\n");
    exit(0);
}

class Pollux_observer : public Xbee_communicator {
    Pollux_configurator& config;

    int meas_idx;
    
    void setup_signal() {
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = sigint_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
    }
    public:
        Pollux_observer(Pollux_configurator& conf) : Xbee_communicator(conf.get_config_option(std::string("tty_port")), 
                                                                       atoi(conf.get_config_option(std::string("wud_sleep_time")).c_str())), config(conf) { 
            Beagle::UART::enable_uart2();
            Beagle::Leds::enable_leds();
            Beagle::Leds::set_status_led();
            this->setup_signal();
            meas_idx=0;
        }
        ~Pollux_observer() {
            Beagle::Leds::disable_leds();
        }

        void get_next_measure(Xbee_result& frame) {
            char* buffer = config.next_measure(frame.get_node_address_as_long());

            printf("buffer to send: %02X, %02X, %02X\n", buffer[0], buffer[1], buffer[2]);
            if (buffer[0] == 0x0)
                printf("   -> skipping measure step\n");
            else {
                printf("   -> sending measure to node %llx\n", frame.get_node_address_as_long());
                send(buffer, frame.get_node_address(), frame.get_network());
            }
            delete(buffer);
        }

        void wake_up() {
            printf("waking up module..\n");
            uint8_t gw_node[] = { 0x00, 0x13, 0xA2, 0x00, 0x40, 0x69, 0x86, 0x75 };

            char high[] = { 0x5, 0x0 };
            char low[] = { 0x4, 0x0 };
            this->send_remote_atcmd(gw_node, 0xFFFF, "D0", high);
            msleep(50);
            this->send_remote_atcmd(gw_node, 0xFFFF, "D0", low);
        }

        void run (XBeeFrame* frame) {

#ifdef VERBOSE
            Xbee_communicator::run(frame); // print frame details
#endif
            Xbee_result payload(frame);

            switch (frame->api_id) {
                case AT_CMD_RESP:
                    //Beagle::Leds::set_rgb_led(Beagle::Leds::GREEN);

                    printf("[AT] Command: '%s' ; Values: '%X'\n", frame->content.at.command, frame->content.at.values);

                    msleep(50);
                    //Beagle::Leds::reset_rgb_led(Beagle::Leds::GREEN);
                    break;
                case RX_PACKET:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::BLUE);

                    payload.print();

                    switch (payload.get_i2c_command()) {
                        case CMD_MEAS:
                            printf("*************** GOT MEASURE\n");
                            config.store_measure(payload);
                            get_next_measure(payload);
                            break;
                        case CMD_INIT:
                            printf("*************** GOT WAKE UP\n");
                            get_next_measure(payload);
                            break;
                        case CMD_HALT:
                            printf("*************** GOT SLEEP DOWN\n");
                            config.push_data(payload.get_node_address_as_long());
                            break;
                        case '*':
                            // just a comment ;)
                            break;
                        default:
                            printf("Unknown Command: %d\n", payload.fmt_i2c_command());
                    }

                    msleep(10);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::BLUE);
                    break;
                case TX_STATUS:
                    if (frame->content.tx.delivery_status != 0x00) {
                        printf("Error sending frame: ");
                        switch (frame->content.tx.delivery_status) {
                            case 0x00: printf("Success\n"); break;
                            case 0x02: printf("CCA Failure\n"); break;
                            case 0x15: printf("Invalid destination endpoint\n"); break;
                            case 0x21: printf("Network ACK Failure\n"); break;
                            case 0x22: printf("Not Joined to Network\n"); break;
                            case 0x23: printf("Self-addressed\n"); break;
                            case 0x24: printf("Address Not Found\n"); break;
                            case 0x25: printf("Route Not Found\n"); break;
                        }
                    }
                default:
                    Beagle::Leds::set_rgb_led(Beagle::Leds::RED);
                    msleep(10);
                    printf("Incoming frame (%02X) that's not useful.\n", frame->api_id);
                    Beagle::Leds::reset_rgb_led(Beagle::Leds::RED);
                    break;
            }
        }
};

// #endif // __XBEE_POLLUX_H__
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// #ifndef __CLI_PARSER_H__
// #define __CLI_PARSER_H__
#include <algorithm>
class Cli_parser {
    int argc;
    char** argv;

    public:
        Cli_parser(int argc, char* argv[]) {
            this->argc = argc;
            this->argv = argv;
        }
        const char* get(const std::string & option) const
        {   
            char ** begin = argv;
            char ** end = argv+argc;
            char ** itr = std::find(begin, end, option);
            if (itr != end && ++itr != end)
            {
                return *itr;
            }
            return (char*)"";
        }

        bool has(const std::string& option) const
        {   
            char ** begin = argv;
            char ** end = argv+argc;
            return std::find(begin, end, option) != end;
        }

};
// #endif // __CLI_PARSER_H__
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    try {
        Cli_parser cli_args(argc, argv);

        if (cli_args.has("-h") || cli_args.has("--help")) {
            std::cout<<"usage: "<<argv[0]<<"[-h] [-p PATH]"<<std::endl\
                <<std::endl\
                <<"Pollux'NZ City Gateway module"<<std::endl\
                <<std::endl\
                <<"optional arguments:"<<std::endl\
                <<"	-p, --path PATH		Path to the configuration directory (default: /etc/pollux)"<<std::endl\
                <<"	-h, --help		This help screen"<<std::endl\
                <<std::endl;
            ::exit(0);
        }

        std::string path_name= "/etc/pollux";

        if (cli_args.has("-p") && !cli_args.has("--path"))
            path_name = cli_args.get("-p");
        else if (!cli_args.has("-p") && cli_args.has("--path"))
            path_name = cli_args.get("--path");
        else if (cli_args.has("-p") && cli_args.has("--path")) {
            std::cerr<<"Can't have both -p or --path. Please choose one. Exiting..."<<std::endl;
            ::exit(1);
        }

        Pollux_configurator pconfig(path_name);

        try {
            pconfig.load_configuration();
            pconfig.load_datastores();
            pconfig.load_sensors();
            pconfig.load_geoloc();
        } catch (Pollux_config_exception pce) {
            std::cerr<<pce.what()<<std::endl;
            std::cerr<<"Can't load configuration, exiting..."<<std::endl;
            ::exit(1);
        }

        Pollux_observer s = Pollux_observer(pconfig);

        if (s.begin() >= 0) {
            for (;;)
                s.poll();
            return 0;
        }
    } catch (std::runtime_error e) {
        std::cerr<<e.what()<<std::endl;
    } catch (std::exception e) {
        std::cerr<<e.what()<<std::endl;
        std::cerr<<"uncaught exception"<<std::endl;
    } catch (...) {
        std::cerr<<"unbelievable exception"<<std::endl;
    }
    return 2;
}

