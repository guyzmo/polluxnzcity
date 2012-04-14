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
 */

#ifndef __LOCAL_H__
#define __LOCAL_H__

#include <pollux/types.h>

#include <sstream>
#include <vector>

int push_to_datastore(std::vector<pollux::string_string_map*>& values_list, pollux::string_string_map& config) {
    std::ostringstream csv_string;

    /// date format
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    if (config.find("post_url") == config.end())
        return -2;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime (buffer,80,"%Y/%m/%d %H:%M:%S",timeinfo);
    csv_string << buffer <<",";
    /// date format

    for (std::vector<pollux::string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it) {
        if ((**val_it)["k"] == "longitude" or (**val_it)["k"] == "latitude" or (**val_it)["k"] == "altitude")
            continue;
        csv_string<<(**val_it)["v"];
        if (val_it+1 != values_list.end())
            csv_string<<",";
        csv_string<<std::endl;
    }

    FILE* fd = fopen(config["post_url"].c_str(),"a");
    if (fd < 0)
        return -1;
    fputs(csv_string.str().c_str(),fd);

    fclose(fd);
    
    return 0;
}
#endif
