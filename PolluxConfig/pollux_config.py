#!/bin/env python


from bottle import install, route, run, PluginError, HTTPError, debug, response, view, static_file, request, TEMPLATE_PATH
from bottle import mako_view as view, mako_template as template

import pollux.static
import pollux.views

from argparse import ArgumentParser
import inspect
import json
import sys
import re

def remove_comments(text):
    """ remove c-style comments.
        text: blob of text with comments (can include newlines)
        returns: text with comments removed
        found on : http://www.saltycrane.com/blog/2007/11/remove-c-comments-python/
    """
    pattern = r"""
                            ##  --------- COMMENT ---------
           /\*              ##  Start of /* ... */ comment
           [^*]*\*+         ##  Non-* followed by 1-or-more *'s
           (                ##
             [^/*][^*]*\*+  ##
           )*               ##  0-or-more things which don't start with /
                            ##    but do end with '*'
           /                ##  End of /* ... */ comment
         |                  ##  -OR-  various things which aren't comments:
           (                ## 
                            ##  ------ " ... " STRING ------
             "              ##  Start of " ... " string
             (              ##
               \\.          ##  Escaped char
             |              ##  -OR-
               [^"\\]       ##  Non "\ characters
             )*             ##
             "              ##  End of " ... " string
           |                ##  -OR-
                            ##
                            ##  ------ ' ... ' STRING ------
             '              ##  Start of ' ... ' string
             (              ##
               \\.          ##  Escaped char
             |              ##  -OR-
               [^'\\]       ##  Non '\ characters
             )*             ##
             '              ##  End of ' ... ' string
           |                ##  -OR-
                            ##
                            ##  ------ ANYTHING ELSE -------
             .              ##  Anything other char
             [^/"'\\]*      ##  Chars which doesn't start a comment, string
           )                ##    or escape
    """
    regex = re.compile(pattern, re.VERBOSE|re.MULTILINE|re.DOTALL)
    noncomments = [m.group(2) for m in regex.finditer(text) if m.group(2)]

    return "".join(noncomments)

class BottlePluginBase(object):
    def __init__(self, keyword):
        self.keyword = keyword
        self.modified = False
        self.api = 2

    def set_modified(self):
        self.modified = True

    def setup(self, app):
        ''' Make sure that other installed plugins don't affect the same
            keyword argument.'''
        for other in app.plugins:
            if not isinstance(other, Configuration): continue
            if other.keyword == self.keyword:
                raise PluginError("Found another plugin with "\
                        "conflicting settings (non-unique keyword: "+self.keyword+").")

    def apply(self, callback, context):
        # Test if the original callback accepts a 'config' keyword.
        # Ignore it if it does not need a database handle.
        args = inspect.getargspec(context.callback)[0]
        if self.keyword not in args:
            return callback
        
        def wrapper(*args, **kwargs):
            print "call wrapper"
            kwargs[self.keyword] = self
            try:
                rv = callback(*args, **kwargs)
                if self.modified:
                    self.save()
                    self.modified = False
            except IOError, e:
                raise HTTPError(500, "Sensors file write error", e)
            return rv

        # Replace the route callback with the wrapped one.
        return wrapper

class Configuration(BottlePluginBase):
    __CONFIG_COMMENT="""/*********************************************************************
                Pollux'NZ City configuration file

This file configures general settings (in configuration section)
and the datastores settings (in datastores section) are used to 
configure where and how to push data to.

in configuration section :
    * tty_port : the serial port character device to be used to
                    communicate with the zigbee module
    * wud_sleep_time : the time the sensor module shall sleep 
                    between two measures

in datastores section :
    * each subsection is the name of the matching datastore module
        (to be included at compilation time, or it will be ignored)
    * in each subsection, the values are used by the datastore module.
        typically: 'post_url' for the address to post to, and 'api_key'
        to sign the data.

This file is generated automatically, please modify it using the
tools given with the pollux'nz city software. Or be very careful
at respecting JSON's syntax.
*********************************************************************/
"""
    def __init__(self, path):
        BottlePluginBase.__init__(self,"pconfig")
        self._path = path
        f = open(path+"config.json")
        try:
            s = remove_comments("".join(f.readlines()))
        finally:
            f.close()
        d = json.loads(s)
        if not "geolocalisation" in d.keys():
            d["geolocalisation"] = {"latitude":"", "longitude":"", "address":""}
        self._geoloc_map = d["geolocalisation"]
        if "configuration" in d and "datastores" in d:
            self._config_map = d["configuration"]
            self._datastores_map = d["datastores"]

    def save(self):
        print "call save"
        try:
            f = open(self._path+"config.json","w")
            f.write(self.__CONFIG_COMMENT)
            f.write(json.dumps(dict(configuration=self._config_map, geolocalisation=self._geoloc_map, datastores=self._datastores_map), sort_keys=True, indent=4))
        finally:
            f.close()
    
    def set_configuration(self, config_d):
        self._config_map = config_d
        self.set_modified()

    def set_datastores(self, dstores_d):
        self._datastores_map = dstores_d
        self.set_modified()

    def set_geoloc(self, geoloc_d):
        self._geoloc_map = geoloc_d
        self.set_modified()

    def get_configuration(self):
        return self._config_map

    def get_datastores(self):
        return self._datastores_map

    def get_geoloc(self):
        return self._geoloc_map

class Sensors(BottlePluginBase):
    __CONFIG_COMMENT="""/*********************************************************************
                Pollux'NZ City sensors file

This file defines the list of sensor modules that are enabled in
each sensor. For each address, shall figure:

    * a map containing either:
        * name, unit, address and register for sensor submodules (e.g. Temperature)
        * name, address and register for action submodules (e.g. Fan)

The data figuring in this file is used to generate the data sent
to the datastores.

This file is generated automatically, please modify it using the
tools given with the pollux'nz city software. Or be very careful
at respecting JSON's syntax.
*********************************************************************/
"""
    def __init__(self, path):
        BottlePluginBase.__init__(self,"sensors")
        self._path = path
        f = open(path+"sensors.json")
        s = remove_comments("".join(f.readlines()))
        self._sensors_map = json.loads(s)
        f = open(path+"sensors_list.json") # TODO curl http://...ckab.net/.../sensors_list
        s = remove_comments("".join(f.readlines()))
        self._sensors_list = json.loads(s)

    def save(self):
        print "call save"
        f = open(self._path+"sensors.json","w")
        f.write(self.__CONFIG_COMMENT)
        f.write(json.dumps(self._sensors_map, sort_keys=True, indent=4))

    def get_sensors(self):
        return self._sensors_map

    def get_sensors_list(self):
        return self._sensors_list

    def set_sensors(self, sensors_d):
        self._sensors_map = sensors_d
        self.set_modified()

@route('/')
@view('accueil')
def index():
    return dict(title="Homepage")

@route('/datas/')
@view('datas')
def datas():
    return dict(title="My Datas")

@route('/sensors/')
@view('sensors')
def get_sensors():
    return dict(title="Sensors",sensors=sensors.get_sensors())

@route('/sensors/', method='POST')
@view('sensors')
def post_sensors():
    for sensor in sensors.get_sensors()[request.forms.get('sensor_addr_old')]:
        sensor["activated"] = False
    for key in request.forms.keys():                       # for each sensor return by form
        if re.match("^0x[0-9]{1,3}_[0-9]{1,3}$",key):      # check if key is a valid I2C + Register addr
            for sensor in sensors.get_sensors()[request.forms.get('sensor_addr_old')]: # get all locals sensors
                if sensor['address'] == key.split('_')[0] and sensor['register'] == key.split('_')[1]: # if posted sensor is the current local sensor
                    sensor['activated'] = True
    result = sensors.get_sensors()
    if request.forms.get('sensor_addr_old') != request.forms.get('sensor_addr'):
        result[request.forms.get('sensor_addr')] = result[request.forms.get('sensor_addr_old')]
        del(result[request.forms.get('sensor_addr_old')])
    #if request.forms.get('longitude') != "" and request.forms.get('latitude') != "":
    #    d = { 'name' : 'longitude',
    #          'value' : request.forms.get('longitude') }
    #    result.append(d)
    #    d = { 'name' : 'latitude',
    #          'value' : request.forms.get('latitude') }
    #    result.append(d)
    sensors.set_sensors(result)
    sensors.save()
    return dict(title="Sensors configuration Saved",sensors=sensors.get_sensors(),welldone=True)

@route('/datastores/')
@view('datastores')
def get_datastores():
    return dict(title="Datastores",datastores=config.get_datastores(),geoloc=config.get_geoloc())

@route('/datastores/', method='POST')
@view('datastores')
def post_datastores():
    result = config.get_datastores()
    geo_map = config.get_geoloc()
    for key in request.forms.keys():
        key_name = "_".join(key.split("_")[1:])
        print key_name
        if key.split("_")[0] == "geo":
            geo_map[key_name] = request.forms.get(key)
        else:
            result[key.split("_")[0]][key_name] = request.forms.get(key)
    config.set_datastores(result)
    config.set_geoloc(geo_map)
    config.save()
    return dict(title="Datastores configuration Saved",datastores=config.get_datastores(),geoloc=config.get_geoloc(),welldone=True)
        
@route('/advanced/')
@view('advanced')
def get_advanced():
    return dict(title="Advanced",config=config._config_map)

@route('/advanced/', method='POST')
@view('advanced')
def post_advanced():
    result = config.get_configuration()
    for key in request.forms.keys():
        result[key] = request.forms.get(key)
    config.set_configuration(result)
    config.save()
    return dict(title="Advanced configuration Saved",config=config._config_map,welldone=True)

import urllib2
@route('/geoloc/<query>')
def get_geoloc(query):
    v = urllib2.urlopen('http://nominatim.openstreetmap.org/search/?format=json&q=%s' % (query,)).read()
    return v

@route('/data/<filename>')
def get_data(filename):
    return static_file(filename, root=config.VARLIB_PATH)

@route('/css/<filename>')
def get_css(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/css/')
	
@route('/img/<filename>')
def get_image(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/img/')

@route('/js/<filename>')
def get_javascript(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/js/')

@route('/sensor/list')
def sensor_list():
    return json.dumps(sensors.get_modules())

@route('/sensor/<addr>')
def sensor_get(addr):
    response.content_type = 'application/json; charset=UTF-8'
    l = list(set([sensor['address'] for sensor in sensors.get_module(addr)]))
    l.sort
    return json.dumps(l)

@route('/sensor/<addr>/<i2c>')
def sensor_get_sensors(addr,i2c):
    response.content_type = 'application/json; charset=UTF-8'
    l = [sensor for sensor in sensors.get_module(addr) if sensor['address'] == i2c]
    return json.dumps(l)
        
@route('/sensor/<addr>/<i2c>/<reg>')
def sensor_get_register(addr,i2c,reg):
    response.content_type = 'application/json; charset=UTF-8'
    l = [sensor for sensor in sensors.get_module(addr) if sensor['address'] == i2c and sensor['register'] == reg]
    if len(l) == 0:
        raise HTTPError(404, "Module at register "+reg+" not found")
    if len(l) > 1:
        raise HTTPError(500, "Error, multiple modules for register "+reg)
    return json.dumps(l[0])

@route('/datastore/list')
def datastore_list():
    response.content_type = 'application/json; charset=UTF-8'
    return json.dumps(config.get_datastores())

@route('/datastore/<name>')
def datastore_get(name):
    response.content_type = 'application/json; charset=UTF-8'
    return json.dumps(config.get_datastore_config(name))

@route('/config/list')
def config_list():
    return config.get_configuration()

@route('/config/<key>')
def config_get(key):
    return config.get_configuration()[key]

def start():
    parser = ArgumentParser(prog=sys.argv[0],
                description="Pollux'NZ City configurator")

    parser.add_argument("-V", '--version', action='version', version="%(prog)s version 0")
    
    parser.add_argument("-D",
                        "--debug",
                        dest="debug",
                        action="store_true",
                        default=False,
                        help="Debug mode")
    parser.add_argument("-p",
                        "--path",
                        dest="path",
                        default=".",
                        help='path to configuration directory. e.g. /etc/pollux/')
    parser.add_argument("-d",
                        "--data",
                        dest="data_path",
                        default="/var/lib/pollux",
                        help='Directory where sensor_values.csv lays')
    # HOST ARGUMENT
    parser.add_argument("-H",
                        "--host",
                        dest="host",
                        default='0.0.0.0',
                        help='Host to serve the web application on.')
    # PORT ARGUMENT
    parser.add_argument("-P",
                        "--port",
                        dest="port",
                        default='8080',
                        help='Port to be used for serving the web application.')
    
    args = parser.parse_args(sys.argv[1:])

    global config
    global sensors

    config = Configuration(args.path+"/")
    sensors = Sensors(args.path+"/")

    TEMPLATE_PATH.insert(0,pollux.views.__path__[0])
    config.VARLIB_PATH=args.data_path

    TEMPLATE_PATH

    install(config)
    install(sensors)
    
    debug(args.debug)
    run(host=args.host, port=args.port, reloader=args.debug)

if __name__ == "__main__":
    start()
