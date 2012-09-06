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

import json
import urllib2

NAME="pachube"
DESC="module that pushes data to cosm/pachube"
DEFAULT_CONFIG = \
{
    "activated": False, 
    "post_url": "http://api.pachube.com/v2/feeds/",
    "feed_id" : "58338",
    "api_key": "XXX"
}

def push_to_pachube(url, key, content):
    opener = urllib2.build_opener(urllib2.HTTPHandler)
    request = urllib2.Request(url, data=content)
    request.add_header('Content-Type', 'application/json')
    request.add_header('X-PachubeApiKey', key)
    request.get_method = lambda: 'PUT'
    try:
        opener.open(request, timeout=5)
    except urllib2.URLError,e:
        print "[Error pushing data in pachube.py: ", e, "]",
        return -1

def push_to_datastore(values_list, config):
    data_str = dict(version="1.0.0",datastreams=[])
    for v in values_list:
        data_str["datastreams"].append(dict(id=v["k"], current_value=v["v"]))
    return push_to_pachube(config["post_url"]+config["feed_id"], config["api_key"],json.dumps(data_str))

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
    sys.exit(test())
