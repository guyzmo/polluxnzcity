#!/bin/env python


from bottle import install, route, run, PluginError, HTTPError, debug, response, view, static_file

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
        if "configuration" in d and "datastores" in d:
            self._config_map = d["configuration"]
            self._datastores_map = d["datastores"]

    def save(self):
        try:
            f = open(self._path+"_config.json","w")
            f.write(self.__CONFIG_COMMENT)
            f.write(json.dumps(dict(configuration=self._config_map, datastores=self._datastores_map), sort_keys=True, indent=4))
        finally:
            f.close()
    
    def get_configuration(self):
        return self._config_map

    def get_datastores(self):
        return self._datastores_map.keys()

    def get_datastore_config(self, name):
        return self._datastores_map[name]

    def add_datastore(self, name, keyval):
        d = json.loads(keyval)
        self._datastores_map[name] = d
        self.set_modified()

    def set_config(self, key, val):
        self._config_map[key] = val
        self.set_modified()

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

    def save(self):
        f = open(self._path+"_sensors.json","w")
        f.write(self.__CONFIG_COMMENT)
        f.write(json.dumps(self._sensors_map, sort_keys=True, indent=4))

    def get_modules(self):
        return self._sensors_map.keys()

    def get_module(self, addr):
        return self._sensors_map[addr]

    def set_module(self, addr, keyval):
        d = json.loads(keyval)
        self._sensors_map[addr] = d
        self.set_modified()



@route('/')
@view('index')
def index():
    return dict(config=config._config_map, datastores=config._datastores_map, sensors=sensors._sensors_map)

@route('/images/<filename:re:.*\.png>#')
def get_image(filename):
    return static_file(filename, root='static/images/', mimetype='image/png')

@route('/js/<filename>')
def get_javascript(filename):
    return static_file(filename, root='static/javascript/')

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

if __name__ == "__main__":
    parser = ArgumentParser(prog=sys.argv[0],
                description="Pollux'NZ City configurator")

    parser.add_argument("-V", '--version', action='version', version="%(prog)s version 0")
    
    parser.add_argument("-d",
                        "--debug",
                        dest="debug",
                        action="store_true",
                        default=False,
                        help="Debug mode")
    parser.add_argument("-P",
                        "--path",
                        dest="path",
                        default=".",
                        help='path to configuration directory. e.g. /etc/pollux/')
    parser.add_argument("-c",
                        "--cache",
                        dest="cache_path",
                        default="/tmp/",
                        help='Directory where all cached objects (session, files) will be stored.\nDefaults to "/tmp/bf/"')
    # HOST ARGUMENT
    parser.add_argument("-H",
                        "--host",
                        dest="host",
                        default='0.0.0.0',
                        help='Host to serve the web application on.')
    # PORT ARGUMENT
    parser.add_argument("-p",
                        "--port",
                        dest="port",
                        default='8080',
                        help='Port to be used for serving the web application.')
    
    args = parser.parse_args(sys.argv[1:])

    config = Configuration(args.path+"/")
    sensors = Sensors(args.path+"/")

    install(config)
    install(sensors)
    
    debug(args.debug)
    run(host=args.host, port=args.port, reloader=args.debug)

