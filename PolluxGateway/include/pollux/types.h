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
 * Toolbox header
 */

#ifndef __POLLUX_TYPES_H__
#define __POLLUX_TYPES_H__

#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string /* target datastore */, std::unordered_map<std::string /* key */, std::string/* val */> > string_string_string_map;
typedef std::unordered_map<std::string /* option */, std::string /* val */> string_string_map;


#endif
