/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 *
 * Pollux'NZ City is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pollux'NZ City is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project. If not, see <http://www.gnu.org/licenses/>.
 */

//#define VERBOSE
//#define NO_ZIGBEE

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

#include <LedWrapper.h>
#include <XBeeWrapper.h>
#include <WireWrapper.h>
#include <PolluxCommunicator.h>

void setup() {
    pinMode(LED_STA, OUTPUT);
    pinMode(LED_ERR, OUTPUT);
    pinMode(LED_XFR, OUTPUT);

#ifdef VERBOSE
    nss.begin(9600);
#endif
#ifdef NO_ZIGBEE
    Serial.begin(9600);
#else
    xbee.begin(9600);
#endif
    Wire.begin();

    Pollux pollux;

    pollux.send_wake_up();
    pollux.wait_for_command();
    pollux.halt();
}

void loop() {
    /* nop */
}

