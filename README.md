
Pollux'NZ City Project
======================

The pollux'NZ City project aims at creating a network of autonomous wireless network
of sensors that uploads the measures on any data silos on Internet.

This repository contains the source code of the first prototype of the project, that
has been deployed  in south of France, with the participation of INRIA Sofia Antipolis,
for a first test run.

Description
===========

The projects is made of two modules:
 * the 'autonomous measuring module', that is made of
   * an arduino with a zigbee radio module ;
   * a battery pack that powers the system ;
   * a solar panel that charges the battery ;
   * an i2c network of sensors.
 * the 'gateway module', that is made of
   * a beaglebone with a zigbee radio module

How it works ?
==============

First, after pluging it in, the user configures the gateway module for his sensor 
network and the interval time between two measures. Then he places the autonomous
sensor module to the best sunlight exposure so the autonomous measuring module can
keep the battery filled up.

The gateway will wait for the given interval time, and waken up the measuring module.
It will then order the measuring module to run the fan, so a free air sample is taken
into the box and run in sequence each sensor to get the measured value. Once all the
sensors have been interrogated, it will tell the measuring module to shut itself down.
Finally, all the measures are given to the enabled plugins that will push the data to
every configured data silo.

Project details
===============

Structure of the project
------------------------

 * Directories

<pre>
PolluxNZCity
    |-- PolluxConfig ............. Contains the configuration plugin (in python)
    |   |-- debian ............... Helper files for package generation
    |   |-- pollux ............... Python module
    |-- PolluxGateway ............ Main program that interfaces with the measuring modules
    |   |-- conf ................. Configuration files that are to be deployed in /etc
    |   |-- extensions ........... Python plugins for uploading data to data silos
    |   |-- include .............. Headers
    |   |-- src .................. Sources
    |   `-- tests ................ Tools that helps testing
    |-- PolluxWakeupduino ........ Main firmware code for the arduino in the measuring module
    |-- PolluxSensorBoard ........ Firmware code for the fan, temperature and noise board
    |-- PolluxSensorDust ......... Firmware code for the dust board
    |-- PolluxSensorSensoric ..... Firmware code for the electrochemical sensoric sensors board
    |-- PolluxSensorTemp ......... Firmware code for the external temperature sensor board
    `-- PolluxContribIpks ........ Helper scripts to build binary packages (for angstrom)
</pre>

 * Branches
   * inria_branch: contains the source code deployed for the INRIA test run
   * i2c_refactoring: non-tested base for a full refactoring of the i2c network

 * Other

In PolluxContribIpks, the package.sh script will run a build and generate the packages
– PolluxConfig and PolluxGateway – that are embedded on the beaglebone. That generator
is now deprecated, the recipes for generating those packages working and being accessible
for the angstrom build system.

Description of the main modules
-------------------------------

* _PolluxConfig_: this module is a web application based on the `bottle.py` module that
  exposes RESTful pathes to the web interface that exposes them using `bootstrap.js`.
  The most important parts are in the `pollux_config` module that contains all the exposed
  routes.
* _PolluxGateway_: this module is the main program that parses the configuration, runs 
  the radio, interrogates every sensor on the distant modules and sends the results
  through every datastore module.
* _PolluxWakeUpduino_: this firmware is the code executed by the embedded arduino that
  is only a gateway between XBee and the i2c network.

The _PolluxGateway_ module has documentation embedded you can generate using `doxygen`,
or by running `make doc` in the directory.

Build and Install
-----------------

* _PolluxConfig_

<pre>
 % cd PolluxConfig
 % python bootstrap.py
 % bin/buildout
 % bin/pollux_config_paster
</pre>

* _PolluxGateway_

first you will need to install the arm-gcc for crosscompilation, or run it directly
on a beaglebone, using native gcc.

<pre>
 % make
 % make doc
 % pollux\_gateway -h
</pre>

for a more verbose version of the software, you can run `make verbose`, that will
create a `pollux\_gateway\_verbose` executable.

Caveats
=======

* The I2C model has been mis-designed. 

We have implemented the i2c stack of the whole project following the way the i2c model
has been implemented in the Arduino and ATTiny wire library suggested ; so we followed
a function calling alike protocol.

But we rapidely found out that this kind of implementation is flawed for our use of 
i2c, as it is not flexible enough and not compatible with most of the i2c modules of
the market. So there's a unfinished implementation of the new i2c handling in the
`i2c_refactoring` branch. That has been not tested at all, and not all the sensors'
firmware has been switched to it.

* The _PolluxGateway_ modules deserves better verbosity

First, instead of compiling two different targets, it should have the common `-v`
switch that makes the executable output more messages. But the verbosity shall also
have levels, so the output is not flooded by the messages.

* The _PolluxGateway_ module shall not configure the MUX values for the xbee cape

We have implemented our first versions of the XBee Cape without the helper EEPROM
that helps the cape autoconfigure the MUX values of the beaglebone. With the new
version of the cape, all the code that configures the MUX settings shall be dropped.
This will finally makes the module work only in userland, and not need to start as
root and drop its privileges once the MUX settings are set.

* The _PolluxGateway_ module and _PolluxWakeUpduino_ firmware shall configure all the zigbee settings

We rely on the manual setting of the XBee modules, to match them up and to configure
their pin ports. That operation is cumbersome, does not work all the time, and needs
to use the X-CTU application under windows.

TODO
====

* finish the refactoring of the i2c stacks
* add more verbosity to the _PolluxGateway_ module
* remove the MUX configuration at _PolluxGateway_ start
* add configuration of the XBee radios in the _PolluxGateway_ module and
  _PolluxWakeUpduino_ firmware.

Copyright
=========

(c)2011-2013 Lucas Fernandez and Bernard Pratz, CKAB, hackable:Devices

Licensed under the GNU GPL version 3

http://polluxnzcity.org/
http://ckab.com/polluxnz-city/
http://hackable-devices.org/projects/project/polluxnzcity/

License
=======

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.




