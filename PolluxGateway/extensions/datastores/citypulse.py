# 
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

import hashlib
import urllib2 
import json

NAME="citypulse"
DESC="upload to citypulse datastore ; http://www.citypulse.org/"

DEFAULT_CONFIG = \
{
    "activated": False, 
    "api_key": "", 
    "post_url": "http://www2.star-apic.com/citypulse/data/SetArduinoData",
    "proxy": ""
}

def calculate_sha1(ostr, key):
    return hashlib.sha1(ostr+key).hexdigest()

def push_to_citypulse(content_string, url, proxy=None):
    req = urllib2.Request(url,
                          headers = {
                              "Content-Type": "text/form-data; charset=utf-8",
                              "Accept": "*/*",   
                              "User-Agent": "polluxnzcity v0.1-beta", 
                              },
                          data = content_string)

    try:
        urllib2.urlopen(req,timeout=5)
        return 1

    except urllib2.URLError,e:
        print "[Error pushing data in citypulse.py: ", e.reason, "]",
        return -1

def push_to_datastore(values_list, config):

    for val in values_list:
        val['v'] = float(val['v'])
        if 'p' in val.keys():
            val['p'] = float(val['p'])

    values_str = json.dumps(values_list)

    post_url = "%s?s=%s&h=%s" % (config["post_url"], 
                                 config["api_key"],
                                 calculate_sha1(values_str, config["api_key"]))

    if "proxy" in config.keys() and not config["proxy"] == "":
        return push_to_citypulse(values_str, post_url, config["proxy"])
    else:
        return push_to_citypulse(values_str, post_url)

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
