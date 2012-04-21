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

/** Defines a Sensor I2C module
 *
 * This is a sensor on the I2C network, defined by what it is expected to measure:
 *      - name: string defining the exposed name of the Sensor
 *      - unit: string defining the unit in what the Sensor is giving the values
 *
 * by how it is addressed:
 *      - address: short unsigned int of the sensor's address on the i2c network
 *      - reg: short unsigned int of the sensor's register to set to 1 for measure
 *
 * and by how to treat the data it outputs:
 *      - length: size of the returned value (starting at register reg+1)
 *      - type: type to cast the measure to once it is read
 */
class Sensor {
    std::string name;
    std::string unit;

    uint8_t address;
    uint8_t reg;

    uint8_t length;
    std::string type;

    protected:
        bool ignored;

    public:
        /// Constructor of the Sensor
        Sensor(std::string name, std::string unit, uint8_t addr, uint8_t reg, uint8_t length, std::string type);
        /// Always returns false, for it to quack like a Sensor
        virtual bool is_ignored();
        /// Getters
        std::string get_name();
        std::string get_unit();
        uint8_t get_address();
        uint8_t get_reg();
        uint8_t get_length();
        std::string get_type();
};

/*** Defines an Action I2C module
 *
 * This is an action on the I2C network, which is defined by the same elements as a Sensor,
 * except we do not expect a value to be returned. If the action has succeeded, it is expected
 * to set the measure register's value to a non-zero value.
 *
 * Using Duck typing, is_ignored() returns always "true", so the value is ignored when processing
 * the measure data for Datastore action.
 */
class Action : public Sensor {
    public:
        /// Constructor of an Action
        Action(std::string name, uint8_t addr, uint8_t reg);
        /// Always returns true, for it to quack like an Action
        bool is_ignored();
};

/// Types to ease hash_map writing in the places it is used.
typedef std::unordered_map<unsigned long long /* ZigBee Address */, std::unordered_map<uint8_t /* i2c addr */, std::vector<Sensor>/* sensor list */ > > long_short_sensor_map;
typedef std::unordered_map<unsigned long long, std::vector<Sensor> > long_sensors_map;

/// A Runtime Exception to be raised when an error is triggered at configuration time.
class Pollux_config_exception : public std::runtime_error {
    public:
        Pollux_config_exception(const std::string& m) : runtime_error(m) {}
};

}

#endif // __POLLUX_CONFIGURATOR_H__

