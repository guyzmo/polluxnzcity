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
#ifndef __POLLUX_PROBER_H__
#define __POLLUX_PROBER_H__

#include <pollux/pollux_configurator.h>

namespace pollux {

/// Class that defines the algorithm to process outgoing and incoming frames to query measures of a Pollux Sensor Zigbee module
class Pollux_prober : public Pollux_configurator {
        
    /// Buffer where values are stored before being pushed
    std::vector<string_string_map*> values_list;

    // iterators
    typedef struct {
        unsigned int meas_idx;
        unsigned int stop;
        std::vector<Sensor>::iterator it;
    } module_iter;
    /// Hashmap of all iterators for every sensor module
    std::unordered_map<unsigned long long, module_iter> module_iterator_map;

    /// hashmap containing iterators for every i2c sensor on each xbee module
    long_short_sensor_map::iterator current_sensor_it;

    public:
        Pollux_prober(std::string& c, std::string& e) : Pollux_configurator(c,e) {}
            
        /// returns the next module to calibrate
        long long unsigned int next_module();

        /** Sets next I2C module and returns data to be set to next measure module
         * @param module for which next measure is to be taken
         * @param inner defines a recursive call
         * @return byte array to send to given module
         */
        char* next_measure(unsigned long long int module, bool inner=false);
        /** Parses and stores the result in a buffer for later processing
         * @param payload Xbee_result data to be parsed
         */
        void store_measure(xbee::Xbee_result& payload);
        
        /** Processes and sends data to all enabled datastores for given module
         * @param module for which to push the data
         */
        void push_data(long long unsigned int module);
};

} // namespace pollux

#endif // __POLLUX_PROBER_H__

