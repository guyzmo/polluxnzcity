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
#ifndef __POLLUX_CONFIGURATOR_H__
#define __POLLUX_CONFIGURATOR_H__

#include <pollux/pollux_sensors.h>
#include <pollux/types.h>
#include <xbee/xbee_result.h>
#include <beaglebone/beaglebone.h>

#include <json/json.h>

#include <malloc.h>
#include <dlfcn.h>

#include <functional>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace pollux {

class Pollux_configurator {

    // string containing the path to configuration directory
    std::string& path_conf;
    // string containing the path to module directory
    std::string& path_ext;

    protected:
        // TODO write accessors for following maps, and put them private
        
        /// Hashmap containing all the sensors for every module indexed by xbee address and register address
        mutable long_short_sensor_map sensors_map;
        /// Hashmap containing all the sensors in an ordered vector indexed by xbee module address
        mutable long_sensors_map sensors_ordered_map;

        /// Hashmap containing the configuration for all datastores
        mutable string_string_string_map datastores_map;
        /// Hashmap containing the general configuration items
        mutable string_string_map configuration_map;
        /// Hashmap containing geographic informations
        mutable string_string_map geoloc_map;

        /// Type alias to ease function callback definition
        typedef int(*datastore_functor_type)(std::vector<std::unordered_map<std::basic_string<char>, std::basic_string<char> >*>&, string_string_map&);
        /// Hashmap containing all the datastores plugin callbacks
        mutable string_string_map datastores_addon_map;

    public:
        Pollux_configurator(std::string& conf, std::string& ext);

        /// configuration accessor
        const std::string& get_config_option(std::string key) const;
        const std::string& get_datastore_value(std::string& ds, std::string& value) const;

        const std::string& get_path_to_extensions() const;

        /// Parses and store configuration
        void load_configuration();
        /// Parses and store geolocalization
        void load_geoloc();
        /// Parses and store datastores
        void load_datastores();
        /// Parses and store sensors
        void load_sensors();

        /// function to be overridden to process next xbee module
        virtual long long unsigned int next_module() = 0;

        /// function to be overridden to process next i2c module for given xbee module
        virtual char* next_measure(unsigned long long int module, bool inner=false) = 0;
        /// function to be overridden to store next measure
        virtual void store_measure(xbee::Xbee_result& payload) = 0;
        /// function to be overridden to push data to datastores
        virtual void push_data(long long unsigned int module) = 0;
};

} // namespace pollux

#endif // __POLLUX_CONFIGURATOR_H__

