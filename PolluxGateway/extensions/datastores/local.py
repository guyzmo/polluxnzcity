 
# Pollux'NZ City source code
# 
# (c) 2012 CKAB / hackable:Devices
# (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
# (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from datetime import datetime

NAME = "local"
DESC = "store measures locally for display in the 'my datas' panel"

DEFAULT_CONFIG = \
{
    "activated": False, 
    "path": "./sensors_data.csv",
}

def push_to_datastore(values_list, config):
    values_str = datetime.strftime(datetime.now(),"%Y/%m/%d %H:%M:%S")
    for val in values_list:
        if not val['k'] in ('latitude','longitude','altitude'): 
            values_str += ","+val['v']
    values_str += "\n"

    try:
        sensors_data = open(config["path"], 'a')
        sensors_data.write(values_str)

        return 1
    except Exception, e:
        print "Error in local.py plugin: ", e
        return -1
    finally:
        sensors_data.close()

def test():
    l = [{'p': '1',   'k': 'Internal temperature', 'u': 'degre celcius', 'v': '42'},
         {'p': '0.1', 'k': 'Temperature',          'u': 'degre celcius', 'v': '41.8'},
         {'p': '3',   'k': 'Noise_Level',          'u': 'dB',            'v': '65'},
         {'p': '5',   'k': 'NO2',                  'u': 'ppm',           'v': '0.0'},
         {'p': '5',   'k': 'CO',                   'u': 'ppm',           'v': '0.0'},
         {'p': '4',   'k': 'Dust',                 'u': 'ug/m3',         'v': '36'}]
    return push_to_datastore(l,DEFAULT_CONFIG)

if __name__ == "__main__":
    import sys
    sys.exit(test)
