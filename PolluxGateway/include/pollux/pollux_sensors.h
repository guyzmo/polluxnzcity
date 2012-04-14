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
#ifndef __POLLUX_SENSORS_H__
#define __POLLUX_SENSORS_H__

#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>

namespace pollux {

class Sensor {
    std::string name;
    std::string unit;

    uint8_t address;
    uint8_t reg;

    protected:
        bool ignored;

    public:
        Sensor(std::string name, std::string unit, uint8_t addr, uint8_t reg);
        std::string get_name();
        std::string get_unit();
        uint8_t get_address();
        uint8_t get_reg();
        virtual bool is_ignored();
};

class Action : public Sensor {
    public:
        Action(std::string name, uint8_t addr, uint8_t reg);
        bool is_ignored();
};

typedef std::unordered_map<unsigned long long /* ZigBee Address */, std::unordered_map<uint8_t /* i2c addr */, std::vector<Sensor>/* sensor list */ > > long_short_sensor_map;
typedef std::unordered_map<unsigned long long, std::vector<Sensor> > long_sensors_map;

class Pollux_config_exception : public std::runtime_error {
    public:
        Pollux_config_exception(const std::string& m) : runtime_error(m) {}
};

}

#endif // __POLLUX_CONFIGURATOR_H__

