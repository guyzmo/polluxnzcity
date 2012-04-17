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

#include <pollux/pollux_sensors.h>
#include <pollux/pollux_configurator.h>

using namespace pollux;

inline bool json_object_has_key(struct json_object* obj, const std::string& key) {
    json_object_object_foreach(obj, k,v) {
        if (std::string(k) == key) 
            return true; 
    }
    return false;
}

Sensor::Sensor(std::string name, std::string unit, uint8_t addr, uint8_t reg) : name(name), unit(unit), address(addr), reg(reg) {
    this->ignored = false;
}
std::string Sensor::get_name() {
    return this->name;
}
std::string Sensor::get_unit() {
    return this->unit;
}
uint8_t Sensor::get_address() {
    return this->address;
}
uint8_t Sensor::get_reg() {
    return this->reg;
}
bool Sensor::is_ignored() {
    return this->ignored;
}

Action::Action(std::string name, uint8_t addr, uint8_t reg) : Sensor(name,"",addr,reg) {
    ignored = true;
}
bool Action::is_ignored() {
    return ignored;
}

Pollux_configurator::Pollux_configurator(std::string& conf, std::string& ext) : path_conf(conf), path_ext(ext) {
        current_sensor_it = sensors_map.begin();
}
const std::string& Pollux_configurator::get_config_option(std::string key) const {
    return configuration_map[key];
}
const std::string& Pollux_configurator::get_datastore_value(std::string& ds, std::string& value) const {
    return datastores_map[ds][value];
}

void Pollux_configurator::load_configuration() {
    std::ostringstream fname;

    fname << path_conf << "/config.json";
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
void Pollux_configurator::load_geoloc() {
    std::ostringstream fname;

    fname << path_conf << "/config.json";
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
void Pollux_configurator::load_datastores() {
    void *handle;
    int (*push_to_datastore)(std::vector<string_string_map*>& values_list, string_string_map& config);
    char *error;

    struct json_object *json_data;
    std::ostringstream fname;

    fname << path_conf << "/config.json";

    json_data = json_object_from_file((char*)fname.str().c_str());
    if (is_error(json_data)) {
        throw Pollux_config_exception("Invalid JSON file. Please check your configuration file.");
    }

    if (!json_object_has_key(json_data,"datastores")) {
        throw Pollux_config_exception("Missing 'datastores' token in configuration file.");
    }

    struct json_object* datastores = json_object_object_get(json_data, "datastores");

    json_object_object_foreach(datastores,name,datastore) {
        std::ostringstream addon_name;
        // find name of the addon
        addon_name<<path_ext<<"/extensions/datastores/"<<name<<".so";

        // open the addon
        handle = dlopen (addon_name.str().c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr<<"WARNING: can't find add-on for "<<name<<std::endl;
            std::cerr<<"         reason: "<<dlerror()<<std::endl;
            continue;
        }

        dlerror();	/* Clear any existing error */

        // get the functor
        push_to_datastore = (datastore_functor_type)dlsym(handle, "push_to_datastore");
        if ((error = dlerror()) != NULL)  {
            std::cerr<<"WARNING: can't load add-on for "<<name<<std::endl;
            std::cerr<<"         reason: "<<error<<std::endl;
            continue;
        }

        // store the functor
        datastores_addon_map[name] = push_to_datastore;

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
void Pollux_configurator::load_sensors() {
    struct json_object *json_data;
    std::ostringstream fname;

    fname << path_conf << "/sensors.json";

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
                    sensors_ordered_map[zb_address].push_back(sensors_map[zb_address][i2c_address].back());
                    // if it is an Action module (i.e. that has *no* 'unit' key)
                } else  {
                    sensors_map[zb_address][i2c_address].push_back(Action(json_object_get_string(json_object_object_get(sensor_module,"name")),
                                i2c_address, reg));
                    sensors_ordered_map[zb_address].push_back(sensors_map[zb_address][i2c_address].back());
#ifdef VERBOSE
                    std::cout<<"module(0x"<<std::setw(16)<<std::setfill('0')<<std::hex<<zb_address<<":0x"<<i2c_address<<") := action("\
                                                                                                <<json_object_get_string(json_object_object_get(sensor_module,"name"))\
                                                                                                <<",0x"<<std::hex<<i2c_address<<",0x"<<std::hex<<reg<<")"<<std::endl;
#endif
                }
            }
        }

        free(json_data);
    }
}

long long unsigned int Pollux_configurator::next_module() {
    if (current_sensor_it == sensors_map.end())
        current_sensor_it = sensors_map.begin();

    module_iterator_map[current_sensor_it->first].it = sensors_ordered_map[current_sensor_it->first].begin();
    module_iterator_map[current_sensor_it->first].meas_idx=0;
    module_iterator_map[current_sensor_it->first].stop=0;

    return (current_sensor_it++)->first;
}

char* Pollux_configurator::next_measure(unsigned long long int module, bool inner) {
    std::vector<Sensor>::iterator& sensor_it = module_iterator_map[module].it;
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

    if (sensor_it == sensors_ordered_map[module].end()) {
        sensor_it = sensors_ordered_map[module].begin();
        buf[0] = CMD_HALT;
        buf[1] = 0x0;
        buf[2] = 0x0;
        debug_printf("************** SENDING HALT COMMAND\n");
        return buf;
    } else if (sensors_map[module][sensor_it->get_address()].size() > 1 and !inner)
        if (module_iterator_map[module].stop == 0) {
            module_iterator_map[module].stop = sensors_map[module][sensor_it->get_address()].size()-1;
            module_iterator_map[module].meas_idx = 0;
        } else if (module_iterator_map[module].meas_idx < module_iterator_map[module].stop) {
            ++(module_iterator_map[module].meas_idx);
            ++sensor_it;
            debug_printf("************** SKIPPING MEASURE\n");
            return buf;
        } else if (module_iterator_map[module].meas_idx == module_iterator_map[module].stop) {
            module_iterator_map[module].stop = 0;
            module_iterator_map[module].meas_idx = 0;
            debug_printf("************** LAST SKIPPING MEASURE\n");

            ++sensor_it;

            free(buf);
            return next_measure(module, true);
        }

    buf[0] = CMD_MEAS;
    buf[1] = sensor_it->get_address();
    buf[2] = sensors_map[module][sensor_it->get_address()].size();

    /*debug_*/printf("    -> measure(s) to send: ");
    for (int i=0;i<sensors_map[module][sensor_it->get_address()].size();++i)
        /*debug_*/printf("%s, ", sensors_map[module][sensor_it->get_address()][i].get_name().c_str());

    if (sensors_map[module][sensor_it->get_address()].size() == 1) 
        ++sensor_it;

    return buf;
}
void Pollux_configurator::store_measure(xbee::Xbee_result& payload) {
    string_string_map* values = new string_string_map();
    std::ostringstream strconv;

    if (sensors_map[payload.get_node_address_as_long()].count(payload.get_i2c_address()) == 0) {
        debug_printf("i2c address %02X is unknown.", payload.get_i2c_address());
        return;
    }
    if (sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].size() <= payload.get_i2c_register()) {
        debug_printf("measure #%d at i2c address %02X is invalid.", payload.get_i2c_register(), payload.get_i2c_address());
        return;
    }
    if (sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).is_ignored()) {
        debug_printf("measure from i2c(%02X,%02X) of type %d is ignored\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
        return;
    }

    switch (payload.get_type()) {
        case I2C_CHR:
            strconv<<payload.get_value_as_char();
            (*values)["k"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
            (*values)["v"] = strconv.str();
            (*values)["u"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
            (*values)["p"] = "0";
            break;
        case I2C_INT:
            strconv<<payload.get_value_as_int();
            (*values)["k"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
            (*values)["v"] = strconv.str();
            (*values)["u"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
            (*values)["p"] = "0";
            break;
        case I2C_FLT:
            strconv<<payload.get_value_as_float();
            (*values)["k"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
            (*values)["v"] = strconv.str();
            (*values)["u"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
            (*values)["p"] = "0";
            break;
        case I2C_DBL:
            strconv<<payload.get_value_as_float();
            (*values)["k"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_name();
            (*values)["v"] = strconv.str();
            (*values)["u"] = sensors_map[payload.get_node_address_as_long()][payload.get_i2c_address()].at(payload.get_i2c_register()).get_unit();
            (*values)["p"] = "0";
            break;
        default:
            printf("    <- measure from i2c(%02X,%02X) of type %d unsupported\n", payload.get_i2c_address(), payload.get_i2c_register(), payload.get_type());
            return;
    }
    values_list.push_back(values);
    printf("    <- measure from i2c(%02X,%02X) ", payload.get_i2c_address(), payload.get_i2c_register());//, json_string.str().c_str());
    return;
}

void Pollux_configurator::push_data(long long unsigned int module) {
    std::ostringstream val_string;

    if (geoloc_map.find("latitude") != geoloc_map.end() and geoloc_map.find("longitude") != geoloc_map.end() ) {
        string_string_map* values = new string_string_map();
        (*values)["k"] = "latitude";
        (*values)["v"] = geoloc_map["latitude"];
        values_list.push_back(values);

        values = new string_string_map();
        (*values)["k"] = "longitude";
        (*values)["v"] = geoloc_map["longitude"];
        values_list.push_back(values);
    }

    if (geoloc_map.find("altitude") != geoloc_map.end()) {
        string_string_map* values = new string_string_map();
        values = new string_string_map();
        (*values)["k"] = "altitude";
        (*values)["v"] = geoloc_map["altitude"];
        values_list.push_back(values);
    }

    for (string_string_string_map::iterator store_it = datastores_map.begin();store_it!=datastores_map.end();++store_it) {
        if (store_it->second["activated"] != "false") {
            if (datastores_addon_map.find(store_it->first) != datastores_addon_map.end()) {
                std::cout<<store_it->first;
                if ((*datastores_addon_map[store_it->first])(values_list, store_it->second) == 0) {
                    std::cout<<"    -> success"<<std::endl;
                    beagle::Leds::set_rgb_led(beagle::Leds::GREEN);
                    msleep(100);
                    beagle::Leds::reset_rgb_led(beagle::Leds::GREEN);
                } else {
                    std::cout<<"    -> failure\n"<<std::endl;
                    beagle::Leds::set_rgb_led(beagle::Leds::RED);
                    msleep(100);
                    beagle::Leds::reset_rgb_led(beagle::Leds::RED);
                }
            } else {
                std::cerr<<"can't find add-on for module: "<<store_it->first<<std::endl;
            }
            
            for (std::vector<string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it)
                delete(*val_it);
            values_list.clear();
        }
    }

}
