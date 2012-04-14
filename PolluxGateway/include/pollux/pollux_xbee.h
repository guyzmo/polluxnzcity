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
 */
#ifndef __XBEE_POLLUX_H__
#define __XBEE_POLLUX_H__

#include <pollux/pollux_configurator.h>
#include <xbee/xbee_communicator.h>
#include <beaglebone/beaglebone.h>

#include <iostream>

#include <signal.h>

void sigint_handler(int c);

namespace pollux {

class Pollux_observer : public xbee::Xbee_communicator {
    Pollux_configurator& config;

    int meas_idx;
    
    void setup_signal();

    public:
        Pollux_observer(Pollux_configurator& conf);
        ~Pollux_observer();

        void get_next_measure(xbee::Xbee_result& frame);

        void wake_up();

        void run (xbee::XBeeFrame* frame);
};

}

#endif // __XBEE_POLLUX_H__

