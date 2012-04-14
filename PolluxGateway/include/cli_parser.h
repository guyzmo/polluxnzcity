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

#ifndef __CLI_PARSER_H__
#define __CLI_PARSER_H__

#include <algorithm>

class Cli_parser {
    int argc;
    char** argv;

    public:
        Cli_parser(int argc, char* argv[]) {
            this->argc = argc;
            this->argv = argv;
        }
        const char* get(const std::string & option) const
        {   
            char ** begin = argv;
            char ** end = argv+argc;
            char ** itr = std::find(begin, end, option);
            if (itr != end && ++itr != end)
            {
                return *itr;
            }
            return (char*)"";
        }

        bool has(const std::string& option) const
        {   
            char ** begin = argv;
            char ** end = argv+argc;
            return std::find(begin, end, option) != end;
        }

};
#endif // __CLI_PARSER_H__

