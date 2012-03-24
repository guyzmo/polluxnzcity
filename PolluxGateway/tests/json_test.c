#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <vector>
#include <string>
#include <sstream>
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
/////////////////////

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
                    //  ZBaddr                      i2caddr         sensors
    std::tr1::unordered_map<unsigned long, std::tr1::unordered_map<uint8_t, std::vector<Sensor>*> > sensors_map;
                        // configopt    val
    std::tr1::unordered_map<std::string, std::string> configuration_map;
                     // datastore                       key             val
    std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > datastores_map;
       

    public:
        int get_configuration(std::string& path) {
            std::ostringstream fname;
            bool checked = false;

            fname << path << "config.json";
            struct json_object *new_obj;

            new_obj = json_object_from_file((char*)fname.str().c_str());
            if (is_error(new_obj)) {
                std::cout<<"Invalid JSON file. Please check your configuration file."<<std::endl;
                return -2;
            }
            if (!json_object_has_key(new_obj,"configuration")) {
                std::cout<<"Missing 'configuration' token in configuration file."<<std::endl;
                return -4;
            }

            struct json_object* config = json_object_object_get(new_obj, "configuration");

            if (!json_object_has_key(config,"tty_port")) {
                std::cout<<"Missing 'tty_port' token in configuration section."<<std::endl;
                return -4;
            }
            if (!json_object_has_key(config,"wud_sleep_time")) {
                std::cout<<"Missing 'wud_sleep_time' token in configuration section."<<std::endl;
                return -4;
            }


            printf("config.tty_port: %s\n", json_object_get_string(json_object_object_get(config,"tty_port")));
            printf("config.wud_sleep_time: %s\n", json_object_get_string(json_object_object_get(config,"wud_sleep_time")));

            free(new_obj);
            return 1;
        }

        bool get_datastores(std::string& path) {
            struct json_object *new_obj;
            std::ostringstream fname;

            fname << path << "config.json";

            new_obj = json_object_from_file((char*)fname.str().c_str());
            if (is_error(new_obj)) {
                std::cout<<"Invalid JSON file. Please check your configuration file syntax."<<std::endl;
                return -2;
            }
        
            if (!json_object_has_key(new_obj,"datastores")) {
                std::cout<<"Missing 'datastores' token in configuration file."<<std::endl;
                return -4;
            }

            struct json_object* datastores = json_object_object_get(new_obj, "datastores");

            json_object_object_foreach(datastores,name,datastore) {
                if (!json_object_has_key(datastore,"post_url")) {
                    std::cout<<"Missing 'post_url' token in datastore "<<name<<" section."<<std::endl;
                    return -4;
                }
                if (!json_object_has_key(datastore,"api_key")) {
                    std::cout<<"Missing 'post_url' token in datastore "<<name<<" section."<<std::endl;
                    return -4;
                }
                printf("datastore(%s).post_url: '%s'\n", name, json_object_get_string(json_object_object_get(datastore,"post_url")));
                printf("datastore(%s).api_key: '%s'\n", name, json_object_get_string(json_object_object_get(datastore,"api_key")));
            }

            free(new_obj);
            return 1;
        }

        int get_sensors(std::string& path) {
           /* 
    std::unordered_map<uint8_t, std::vector<Sensor>*> sensors_map;
    std::vector<Sensor>* sensor;

    sensor = new std::vector<Sensor>();
    sensor->push_back(Sensor("Dust","ppm",0x26, 0));
    sensors_map[0x26] = sensor;

    sensor = new std::vector<Sensor>();
    sensor->push_back(Action("Fan",0x27, 0));
    sensor->push_back(Sensor("Noise level","dB",0x27, 1));
    sensor->push_back(Sensor("Temperature","ºC", 0x27, 2));
    sensors_map[0x27] = sensor;
          */
            struct json_object *new_obj;
            std::ostringstream fname;
            bool checked = false;

            fname << path << "sensors.json";

            new_obj = json_object_from_file((char*)fname.str().c_str());

            new_obj = json_object_from_file((char*)fname.str().c_str());
            if (is_error(new_obj)) {
                std::cout<<"Invalid JSON file. Please check your configuration file syntax."<<std::endl;
                return -2;
            }
            
            json_object_object_foreach(new_obj,module_name,module) {
                for (int i=0;i<json_object_array_length(module);++i) {
                    json_object_object_foreach(json_object_array_get_idx(module,i),sensor_key,sensor_val) {
                        printf("module(%s)->sensor(%d):	%s: %s\n", module_name, i, sensor_key, json_object_to_json_string(sensor_val));
                    }
                }
            }

            free(new_obj);
            return 1;
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

    return 0;
}

