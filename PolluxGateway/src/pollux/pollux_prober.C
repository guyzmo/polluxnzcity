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

#include <pollux/pollux_prober.h>

using namespace pollux;

Pollux_prober::Pollux_prober(std::string& conf, std::string& ext) : Pollux_configurator(conf,ext) {
    current_sensor_it = sensors_map.begin();
}

long long unsigned int Pollux_prober::next_module() {
    if (current_sensor_it == sensors_map.end())
        current_sensor_it = sensors_map.begin();

    module_iterator_map[current_sensor_it->first].it = sensors_ordered_map[current_sensor_it->first].begin();
    module_iterator_map[current_sensor_it->first].meas_idx=0;
    module_iterator_map[current_sensor_it->first].stop=0;

    return (current_sensor_it++)->first;
}

char* Pollux_prober::next_measure(unsigned long long int module, bool inner) {
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
void Pollux_prober::store_measure(xbee::Xbee_result& payload) {
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

void Pollux_prober::push_data(long long unsigned int module) {
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
        }
    }
    
    for (std::vector<string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it)
        delete(*val_it);
    values_list.clear();

}
