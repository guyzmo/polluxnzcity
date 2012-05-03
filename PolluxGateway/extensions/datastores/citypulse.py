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
import pycurl
import json

DEFAULT_CONFIG = \
{
    "citypulse": {
        "activated": False, 
        "api_key": "", 
        "post_url": "http://www2.star-apic.com/citypulse/data/SetArduinoData",
        "proxy": ""
    }
}

def calculate_sha1(ostr, key):
    return hashlib.sha1(ostr+key).hexdigest()

def push_to_citypulse(content_string, url, proxy=None):
    curl = pycurl.Curl()
    curl.setopt(pycurl.URL, url)
    if proxy:
        curl.setopt(pycurl.PROXY, proxy)
    curl.setopt(pycurl.POST, 1)
    curl.setopt(pycurl.POSTFIELDS, content_string)
    curl.setopt(pycurl.HTTPHEADER, ["Content-Type: text/form-data; charset=utf-8"])
    curl.perform()
    if curl.getinfo(pycurl.HTTP_CODE) == 200:
        return 1
    else:
        return -1

def push_to_datastore(values_list, config):

    for val in values_list:
        val['v'] = float(val['v'])
        val['p'] = float(val['p'])

    values_str = json.dumps(values_list)

    post_url = "%s?s=%s&h=%s" % (config["post_url"], 
                                 config["api_key"],
                                 calculate_sha1(values_str, config["api_key"]))

    if "proxy" in config:
        return push_to_citypulse(values_str, post_url)
    else:
        return push_to_citypulse(values_str, post_url, config["proxy"])


if __name__ == "__main__":
    c = DEFAULT_CONFIG[DEFAULT_CONFIG.keys()[0]]
    l = [{'p': '0.1', 'k': 'temp', 'u': 'degre celcius', 'v': '42'},
         {'p': '0.0001', 'k': 'dust', 'u': 'ppm', 'v': '0.001'}]
    exit(push_to_datastore(l,c))
