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

#include <pollux/pollux_extension.h>

namespace "pollux" {

/** This class is a facility for the Pollux application to
 * load and run python modules */
class PolluxExtension {
    std::string append_python_path;

    public:
    /** Initializes the python environment to load the extensions 
     *
     * @param path: string containing the path to the extension directory
     *
     * */
    PolluxExtension(std::string& path);
    /** stops the python environment */
    ~PolluxExtension();

    /** executes the module module 
     *
     * @param module: string containing the name of the module
     * @param configuration_map: configuration map matching the module to be launched
     * @param values_list: list of maps containing all the measures
     * @return integer: <0 module error, >0 success, =0 couldn't load module
     *
     * the name of the module shall match configuration file's 
     * datastore item and the name of the python module in the
     * extension's directory
     *
     * the measures are a list of maps having the following nomenclature:
     * "k" : "name of the measure" [string]
     * "u" : "unit of the measure" [string]
     * "v" : "value of the measure" [string containing float]
     * "p" : "precision of the measure" [string containing float]
     */
    int push_to_datastore(std::string& module, 
                            string_string_map configuration_map,
                            std::vector<string_string_map*> values_list);

}
}
