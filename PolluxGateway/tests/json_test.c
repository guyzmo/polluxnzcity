#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <tr1/unordered_map>

////////////////////// JSON_WRAPPER
#include <json/json.h>
inline bool json_object_has_key(struct json_object* obj, const std::string& key) {
    json_object_object_foreach(obj, k,v) {
        if (std::string(k) == key) 
            return true; 
    }
    return false;
}
//////////////////////

////////////////////// CLI_OPT
#include <algorithm>
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return (char*)"";
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}
///////////////////// Sensor

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

////////////////////

class PolluxConfiguration {
    std::tr1::unordered_map<unsigned long /* ZigBee Address */, std::tr1::unordered_map<uint8_t /* i2c addr */, std::vector<Sensor>/* sensor list */ > > sensors_map;
    std::tr1::unordered_map<std::string /* target datastore */, std::tr1::unordered_map<std::string /* key */, std::string/* val */> > datastores_map;
    std::tr1::unordered_map<std::string /* config option */   , std::string /* val */> configuration_map;
       
    unsigned int meas_idx;
    unsigned int stop;
    std::tr1::unordered_map<uint16_t, std::tr1::unordered_map<uint8_t, std::vector<Sensor> >::iterator > module_iterator_map;

    public:
        PolluxConfiguration() {
            meas_idx=-1;
            stop=0;
        }
        int get_configuration(std::string& path) {
            std::ostringstream fname;

            fname << path << "config.json";
            struct json_object *json_data;

            json_data = json_object_from_file((char*)fname.str().c_str());
            if (is_error(json_data)) {
                std::cout<<"Invalid JSON file. Please check your configuration file."<<std::endl;
                return -2;
            }
            if (!json_object_has_key(json_data,"configuration")) {
                std::cout<<"Missing 'configuration' token in configuration file."<<std::endl;
                return -4;
            }

            struct json_object* config = json_object_object_get(json_data, "configuration");

            if (!json_object_has_key(config,"tty_port")) {
                std::cout<<"Missing 'tty_port' token in configuration section."<<std::endl;
                return -4;
            }
            if (!json_object_has_key(config,"wud_sleep_time")) {
                std::cout<<"Missing 'wud_sleep_time' token in configuration section."<<std::endl;
                return -4;
            }


            json_object_object_foreach(config,key,value) {
                configuration_map[key] = json_object_get_string(json_object_object_get(config,key));
                std::cout<<"config."<<key<<": "<<json_object_get_string(json_object_object_get(config,key))<<std::endl;
            }

            free(json_data);
            return 1;
        }

        bool get_datastores(std::string& path) {
            struct json_object *json_data;
            std::ostringstream fname;

            fname << path << "config.json";

            json_data = json_object_from_file((char*)fname.str().c_str());
            if (is_error(json_data)) {
                std::cout<<"Invalid JSON file. Please check your configuration file syntax."<<std::endl;
                return -2;
            }
        
            if (!json_object_has_key(json_data,"datastores")) {
                std::cout<<"Missing 'datastores' token in configuration file."<<std::endl;
                return -4;
            }

            struct json_object* datastores = json_object_object_get(json_data, "datastores");

            json_object_object_foreach(datastores,name,datastore) {
                if (!json_object_has_key(datastore,"post_url")) {
                    std::cout<<"Missing 'post_url' token in datastore "<<name<<" section."<<std::endl;
                    return -4;
                }
                if (!json_object_has_key(datastore,"api_key")) {
                    std::cout<<"Missing 'post_url' token in datastore "<<name<<" section."<<std::endl;
                    return -4;
                }
                json_object_object_foreach(datastore,key,value) {
                    printf("datastore(%s).%s: '%s'\n", name, key, json_object_get_string(json_object_object_get(datastore,key)));
                    datastores_map[name][key] = json_object_get_string(value);
                }
            }

            free(json_data);
            return 1;
        }

        int get_sensors(std::string& path) {
            struct json_object *json_data;
            std::ostringstream fname;

            fname << path << "sensors.json";

            json_data = json_object_from_file((char*)fname.str().c_str());

            json_data = json_object_from_file((char*)fname.str().c_str());

            if (is_error(json_data)) {
                std::cout<<"Invalid JSON file. Please check your configuration file syntax."<<std::endl;
                return -2;
            }
            
            json_object_object_foreach(json_data,module_name,module) {
                unsigned long int zb_address;
                std::istringstream* ss = new std::istringstream( module_name );
                (*ss) >> std::hex >> zb_address;
                delete(ss);

                // if sensor declaration has name, address and register declared
                for (int i=0;i<json_object_array_length(module);++i) {
                    struct json_object* sensor_module = json_object_array_get_idx(module, i);

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
                            std::cout<<"module(0x"<<std::setw(16)<<std::setfill('0')<<std::hex<<zb_address<<":0x"<<i2c_address<<") := sensor("\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"name"))<<","\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"unit"))\
                                                                                                       <<",0x"<<std::hex<<i2c_address<<",0x"<<std::hex<<reg<<")"<<std::endl;
                            sensors_map[zb_address][i2c_address].push_back(Sensor(json_object_get_string(json_object_object_get(sensor_module,"name")),
                                        json_object_get_string(json_object_object_get(sensor_module,"unit")),
                                        i2c_address, reg));
                            // if it is an Action module (i.e. that has *no* 'unit' key)
                        } else  {
                            sensors_map[zb_address][i2c_address].push_back(Action(json_object_get_string(json_object_object_get(sensor_module,"name")),
                                        i2c_address, reg));
                            std::cout<<"module(0x"<<std::setw(16)<<std::setfill('0')<<std::hex<<zb_address<<":0x"<<i2c_address<<") := action("\
                                                                                                       <<json_object_get_string(json_object_object_get(sensor_module,"name"))\
                                                                                                       <<",0x"<<std::hex<<i2c_address<<",0x"<<std::hex<<reg<<")"<<std::endl;
                        }
                    }
                    module_iterator_map[zb_address] = sensors_map[zb_address].begin();
                }

                free(json_data);
                return 1;
            }
        }
        

        char* get_next_measure(unsigned long int module) {
            std::tr1::unordered_map<uint8_t, std::vector<Sensor> >::iterator& sensor_it = module_iterator_map[module];
            char* buf = (char*)malloc(sizeof(char)*3);
            buf[0] = 0x0;
            buf[1] = 0x0;
            buf[2] = 0x0;

            if (sensor_it->second.size() > 1 and stop == 0) {
                stop = sensor_it->second.size()-2;
                meas_idx = 0;
            } else if (meas_idx < stop) {
                ++meas_idx;
                return buf;
            } else if (meas_idx == stop) {
                stop = 0;
                meas_idx = -1;
                return buf;
            }

            buf[0] = 0x42;
            buf[1] = (unsigned short int)(*sensor_it).first;
            buf[2] = (unsigned short int)(*sensor_it).second.size();

            ++sensor_it;
            if (sensor_it == sensors_map[module].end()) {
                sensor_it = sensors_map[module].begin();
            }

            return buf;
        }
};

int main(int argc, char * argv[])
{
    PolluxConfiguration pconfig;

    if(cmdOptionExists(argv, argv+argc, std::string("-h")))
    {
        std::cout<<"Pollux'NZ City Gateway v0.0"<<std::endl;
        std::cout<<argv[0]<<" [-h|-c configuration_directory]"<<std::endl;
        return 0;
    }

    std::string filename(getCmdOption(argv, argv + argc, std::string("-c")));

    if (!pconfig.get_configuration(filename))
        std::cout<<"Configuration file not found. Can't get configuration."<<std::endl;
    if (!pconfig.get_datastores(filename))
        std::cout<<"Configuration file not found. Can't get datastores"<<std::endl;
    if (!pconfig.get_sensors(filename))
        std::cout<<"Sensor description file not found."<<std::endl;

    char* buf;
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);
    buf = pconfig.get_next_measure(0x0013a20040698679); std::cout<<"sending: "<<std::hex<<(int)buf[0]<<","<<std::hex<<(int)buf[1]<<","<<std::hex<<(int)buf[2]<<std::endl;delete(buf);

    return 0;
}

