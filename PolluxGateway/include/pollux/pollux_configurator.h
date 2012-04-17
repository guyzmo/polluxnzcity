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

#include <dlfcn.h>

#include <functional>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace pollux {

class Pollux_configurator {
    protected:
        mutable long_short_sensor_map sensors_map;
        mutable long_sensors_map sensors_ordered_map;
        mutable string_string_string_map datastores_map;
        mutable string_string_map configuration_map;
        mutable string_string_map geoloc_map;

        typedef int(*datastore_functor_type)(std::vector<std::unordered_map<std::basic_string<char>, std::basic_string<char> >*>&, string_string_map&);
        mutable std::unordered_map<std::string, datastore_functor_type> datastores_addon_map;
        
        std::vector<string_string_map*> values_list;

        // iterators
        typedef struct {
            unsigned int meas_idx;
            unsigned int stop;
            std::vector<Sensor>::iterator it;
        } module_iter;
        std::unordered_map<unsigned long long, module_iter> module_iterator_map;

        long_short_sensor_map::iterator current_sensor_it;

        std::string& path_conf;
        std::string& path_ext;

    public:
        Pollux_configurator(std::string& conf, std::string& ext);
        const std::string& get_config_option(std::string key) const;
        const std::string& get_datastore_value(std::string& ds, std::string& value) const;

        void load_configuration();
        void load_geoloc();
        void load_datastores();
        void load_sensors();

        virtual long long unsigned int next_module() = 0;

        virtual char* next_measure(unsigned long long int module, bool inner=false) = 0;
        virtual void store_measure(xbee::Xbee_result& payload) = 0;
        
        virtual void push_data(long long unsigned int module) = 0;
};

} // namespace pollux

#endif // __POLLUX_CONFIGURATOR_H__

