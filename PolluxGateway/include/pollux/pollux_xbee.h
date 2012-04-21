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

namespace pollux {

/** This class configures how the polling algorithm of the gateway works
 * 
 * Its responsibility is to handle the whole XBee networking protocol:
 *  at construction:
 *      * it enables the Beaglebone's UART
 *      * it enables the Beaglebone's Leds
 *      * it puts the status LED to on
 *
 *  at destruction:
 *      * it disables the Beaglebone's UART
 *      * it disables the Beaglebone's Leds
 *
 *  on poll() call:
 *      * if timeout happens, wake_up() is called:
 *          * configure config to take the "next module" to work with
 *          * which sends a PIN change on XBee's D0 to trigger the arduino's wake up interruption
 *      * if a frame is received, run() is called:
 *          * on packet receive:
 *              - if a wake up response is received, ask for the next measure from config to be sent to XBee
 *              - if a measure response is received, from config store it and ask the next one to be sent to XBee
 *              - if a halt response is received, trigger config's push_data so stored values are sent where it belongs
 *
 */
class Pollux_observer : public xbee::Xbee_communicator {
    Pollux_configurator& config;

    public:
        Pollux_observer(Pollux_configurator& conf);
        ~Pollux_observer();

        /** get next measure from config and ask for it on XBee network
         * @param frame: a frame reference, which will be set with the XBee frame's values
         */
        void get_next_measure(xbee::Xbee_result& frame);

        /// defines what to do on timeouts of polling
        void wake_up();

        /// defines the main behaviours of the application
        /// on received packets
        void run (xbee::XBeeFrame* frame);
};

}

#endif // __XBEE_POLLUX_H__

