#!/bin/env python


from pollux.bottle import install, route, run, PluginError, HTTPError, debug, response, static_file, request, TEMPLATE_PATH
from pollux.bottle import mako_view as view, mako_template as template

import pollux.static
import pollux.views

from argparse import ArgumentParser
import inspect
import json
import sys
import imp
import os
import re
import gc

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

class PolluxPluginBase(BottlePluginBase):
    def __init__(self, name):
        BottlePluginBase.__init__(self,name)
        self._parse_error = None

    def reload_maps(self, path, map_names):
        try:
            for name in map_names:
                self.load_json(path, name)
            self._parse_error = None
        except Exception, err:
            self._parse_error = err

    def load_json(self, path, name):
        """
        load a json file from %path%/%name%+".json" and store it into self._%name% in current object
        """
        try:
            f = open(path+name+".json", "r")
            s = remove_comments("".join(f.readlines()))
            setattr(self, "_"+name+"_map", json.loads(s))
        finally:
            f.close()

    def store_json(self, name):
        try:
            f = open(self._path+name+".json","w")
            f.write(self.COMMENT)
            f.write(json.dumps(getattr(self, "_"+name+"_map"), sort_keys=True, indent=4))
        finally:
            f.close()

    def get_error(self):
        return self._parse_error

class Configuration(PolluxPluginBase):
    COMMENT="""/*********************************************************************
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

in geolocalisation section :
    * defines the latitude, longitude, altitude and address of the device

This file is generated automatically, please modify it using the
tools given with the pollux'nz city software. Or be very careful
at respecting JSON's syntax.
*********************************************************************/
"""
    def __init__(self, path):
        PolluxPluginBase.__init__(self,"config")
        self._path = path
        self.reload_config()

    def reload_config(self):
        self.reload_maps(self._path, ["config"])

    def save(self):
        print "call save"
        self.store_json("config")
    
    def set_configuration(self, config_d):
        self._config_map["configuration"] = config_d
        self.set_modified()

    def set_datastores(self, dstores_d):
        self._config_map["datastores"] = dstores_d
        self.set_modified()

    def set_geoloc(self, geoloc_d):
        self._config_map["geolocalisation"] = geoloc_d
        self.set_modified()

    def get_configuration(self):
        return self._config_map["configuration"]

    def get_datastores(self):
        return self._config_map["datastores"]

    def get_geoloc(self):
        return self._config_map["geolocalisation"]

class Sensors(PolluxPluginBase):
    COMMENT="""/*********************************************************************
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
        self.reload_config()

    def reload_config(self):
        self.reload_maps(self._path, ["sensors", "sensors_list"])

    def save(self):
        print "call save"
        self.store_json("sensors")

    def get_sensors(self):
        return self._sensors_map

    def get_sensors_list(self):
        return self._sensors_list_map

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
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    return dict(title="Sensors",sensors=sensors)

@route('/sensors/', method='POST')
@view('sensors')
def post_sensors():
    # XXX TODO add support for multiple sensor modules
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())

    form_keys = [key for key in request.forms.keys() if re.match("^0x[0-9]{1,3}_[0-9]{1,3}$",key)]

    sensors_list = []
    for sensor in sensors.get_sensors_list():
        sensor = sensor.copy()
        if sensor["address"] + "_" + sensor["register"] in form_keys:
            sensor["activated"] = True
        sensors_list.append(sensor)

    result = sensors.get_sensors()
    result[request.forms.get('sensor_addr')] = sensors_list
    if request.forms.get('sensor_addr_old') != request.forms.get('sensor_addr'):
        del(result[request.forms.get('sensor_addr_old')])

    sensors.set_sensors(result)
    sensors.save()
    return dict(title="Sensors configuration Saved", message="Configuration successfully saved.",sensors=sensors,welldone=True)

@route('/datastores/')
@view('datastores')
def get_datastores():
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    return dict(title="Datastores",datastores=config.get_datastores(),geoloc=config.get_geoloc())

@route('/datastores/', method='POST')
@view('datastores')
def post_datastores():
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    result = config.get_datastores()
    for n,kv in result.iteritems():
        kv["activated"] = False
        
    geo_map = config.get_geoloc()
    for key in request.forms.keys():
        key_name = "_".join(key.split("_")[1:])
        if key.split("_")[0] == "geo":
            geo_map[key_name] = request.forms.get(key)
        else:
            if key_name == "activated":
                result[key.split("_")[0]][key_name] = True
            else:
                result[key.split("_")[0]][key_name] = request.forms.get(key)
    config.set_datastores(result)
    config.set_geoloc(geo_map)
    config.save()
    return dict(title="Datastores configuration Saved", message="Configuration successfully saved.",datastores=config.get_datastores(),geoloc=config.get_geoloc(),welldone=True)
        
@route('/advanced/')
@view('advanced')
def get_advanced():
    return dict(title="Advanced",config=config.get_configuration())

@route('/advanced/', method='POST')
@view('advanced')
def post_advanced():
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    result = config.get_configuration()
    for key in request.forms.keys():
        result[key] = request.forms.get(key)
    config.set_configuration(result)
    config.save()
    return dict(title="Advanced configuration Saved", message="Configuration successfully saved.",config=config.get_configuration(),welldone=True)

import urllib2
@route('/geoloc/<query>')
def get_geoloc(query):
    v = urllib2.urlopen('http://nominatim.openstreetmap.org/search/?format=json&q=%s' % (query,)).read()
    return v

@route('/data/csv')
def get_data():
    path = os.path.split(config.get_datastores()["local"]["path"])[0]
    filename = os.path.split(config.get_datastores()["local"]["path"])[-1]
    return static_file(filename, root=path)

@route('/css/<filename>')
def get_css(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/css/')
	
@route('/img/<filename>')
def get_image(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/img/')

@route('/js/<filename>')
def get_javascript(filename):
    return static_file(filename, root=pollux.static.__path__[0]+'/js/')

import urllib2
@route('/sensors/reload')
@view('advanced')
def sensors_reload():
    response = urllib2.urlopen("http://www.polluxnzcity.net/beta/sensors_list.json")
    try:
        sensors_list_str = response.read()

        try:
            f = open(sensors._path+"sensors_list.json","w")
            f.write(sensors_list_str)
        finally:
            f.close()
        sensors.reload_config()
        if sensors.get_error():
            raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
        return dict(title="Configuration reloaded", message="Sensor's list successfully upgraded",config=config.get_configuration(),welldone=True)

    except Exception, err:
        return dict(title="Configuration reloaded", message="Sensor's list failed to upgrade: "+err,config=config.get_configuration(),failed=True)

@route('/sensor/list')
def sensor_list():
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    return json.dumps(sensors.get_modules())

@route('/sensor/<addr>')
def sensor_get(addr):
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    response.content_type = 'application/json; charset=UTF-8'
    l = list(set([sensor['address'] for sensor in sensors.get_module(addr)]))
    l.sort
    return json.dumps(l)

@route('/sensor/<addr>/<i2c>')
def sensor_get_sensors(addr,i2c):
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    response.content_type = 'application/json; charset=UTF-8'
    l = [sensor for sensor in sensors.get_module(addr) if sensor['address'] == i2c]
    return json.dumps(l)
        
@route('/sensor/<addr>/<i2c>/<reg>')
def sensor_get_register(addr,i2c,reg):
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    response.content_type = 'application/json; charset=UTF-8'
    l = [sensor for sensor in sensors.get_module(addr) if sensor['address'] == i2c and sensor['register'] == reg]
    if len(l) == 0:
        raise HTTPError(404, "Module at register "+reg+" not found")
    if len(l) > 1:
        raise HTTPError(500, "Error, multiple modules for register "+reg)
    return json.dumps(l[0])

@route('/datastore/list')
def datastore_list():
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    response.content_type = 'application/json; charset=UTF-8'
    return json.dumps(config.get_datastores())

@route('/datastore/<name>')
def datastore_get(name):
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    response.content_type = 'application/json; charset=UTF-8'
    return json.dumps(config.get_datastore_config(name))

@route('/config/list')
def config_list():
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    return config.get_configuration()

@route('/config/<key>')
def config_get(key):
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    return config.get_configuration()[key]

@route('/config/reload')
@view('advanced')
def config_reload():
    sensors.reload_config()
    if sensors.get_error():
        raise HTTPError(500, "Configuration error, sensors JSON file is corrupted<br /> "+sensors.get_error())
    config.reload_config()
    if config.get_error():
        raise HTTPError(500, "Configuration error, configuration JSON file is corrupted<br /> "+config.get_error())
    return dict(title="Configuration reloaded", message="Configuration successfully reloaded",config=config.get_configuration(),welldone=True)

@route('/system/restart')
@view('advanced')
def restart_service():
    try:
        import subprocess
        subprocess.check_call(['/usr/bin/sudo', '/bin/systemctl', 'restart', 'pollux_gateway.service'])
        return dict(title="Service restarted",config=config.get_configuration(),message="Service successfully restarted.",welldone=True)
    except:
        return dict(title="Service restart failure.",message="Couldn't restart service. Check logs.",config=config.get_configuration(),failed=True)
    
@route('/system/module/upload', method="POST")
@view('advanced')
def upload_module():
    name = request.forms.name
    module = request.files.module
    try:
        if name and module and module.file:
            filename = module.filename
            code = module.file.read()

            pymodule = imp.new_module(name)
            exec(code,pymodule.__dict__)

            if not "DEFAULT_CONFIG" in pymodule.__dict__:
                raise Exception("Missing DEFAULT_CONFIG dictionary in global of "+filename)
            elif not "push_to_datastore" in pymodule.__dict__:
                raise Exception("Missing push_to_datastore() function in "+filename)
            else:
                try:
                    fout = open(config.USRLIB_PATH+"/extensions/datastores/"+name+".py","w")
                    fout.write(code)
                    config.get_datastore()[name] = pymodule.DEFAULT_CONFIG
                    config.save()
                finally:
                    fout.close()
        else:
            raise Exception("Form has not been correctly filled in. Try again !")
        return dict(title="Module loaded.", message="Module loaded, you can now <a href='/datastores/'>configure it</a>",config=config.get_configuration(),welldone=True)
    except Exception, err:
        return dict(title="Failure to load module.", message="Failed to load module: "+str(err),config=config.get_configuration(),failed=True)

@route('/system/logs')
@view('logs')
def view_logs():
    try:
        f = open('/var/log/messages','r')
        return dict(title="Log Viewer",logs=f.readlines()[-100:])
    except Exception:
        raise HTTPError(500, "Feature not yet implemented. Really sorry.")
    finally:
        logs = None
        gc.collect()
        f.close()

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
                        default="/etc/pollux",
                        help='path to configuration directory. e.g. /etc/pollux/')
    parser.add_argument("-l",
                        "--lib",
                        dest="lib_path",
                        default="/usr/lib/pollux",
                        help='Directory where the modules lay')
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
    config.USRLIB_PATH=args.lib_path

    install(config)
    install(sensors)

    return args
    
def run_app():
    args = start()
    debug(args.debug)
    run(host=args.host, port=args.port, reloader=args.debug)

def get_lighttpd_configuration():
    print """
server.modules += ("mod_fastcgi", "mod_rewrite")

fastcgi.server = (
    "/index.py" =>
       (
            "python-fcgi" =>
                (
                    "socket" => "/tmp/fastcgi.python.socket",
                    "bin-path" => "/usr/bin/python %(POLLUX_CONFIG_MODULE_FULLPATH)s -p /etc/pollux -l /usr/lib/pollux",
                    "check-local" => "disable",
                    "max-procs" => 1,
                )
        )
)

url.rewrite-once = (
    "^/(.*)$" => "/index.py/$1"
)
""" % {"POLLUX_CONFIG_MODULE_FULLPATH" : os.path.abspath(__file__) }

def make_app():
    start()
    from flup.server.fcgi import WSGIServer
    WSGIServer(pollux.bottle.default_app()).run()

if __name__ == "__main__":
    make_app()

