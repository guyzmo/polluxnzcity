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
 * resources that helped:
 * http://www.circuitsathome.com/mcu/playing-xbee-part-4-api
 * http://www.chasingtrons.com/main/2010/11/13/xbee-propeller-chip.html
 *
 * Xbee Light Communicator Library
 */

#ifndef __XBEE_COMM_H__
#define __XBEE_COMM_H__

#include <xbee/toolbox.h>
#include <xbee/xbee_types.h>
#include <xbee/xbee_constants.h>

#include <beaglebone/beagle_serial.h>

#include <string.h>

// --------------------------------------------------------------- XBEE Lib
namespace xbee {

class Xbee_communicator : public beagle::Serial {

    int frm_id;

    void xmit_req(uint8_t* addr64, uint16_t network, uint8_t nbytes, uint8_t* data, uint8_t frameid, uint8_t bradius, uint8_t options);
    void send_atcmd(const char* at_command, const char* param_value);
    int rcpt_frame(XBeeFrame* frame);

    protected:
        void print_data(uint8_t* data, uint16_t len, int type) const;
        void print_frame(XBeeFrame* frame) const;
        void send_remote_atcmd(uint8_t* addr64, uint16_t network, const char* at_command, const char* param_value);
        void send_remote_atcmd(uint64_t addr64, uint16_t network, const char* at_command, const char* param_value);
        
    public:
        Xbee_communicator(const std::string& port, int poll_wait);

        int begin (int speed);

        int read(bool no_esc=false);
        ssize_t write(uint8_t i);

        void send(char* payload);
        void send(char* payload, uint8_t* node, uint16_t net);
        void send(char* payload, uint64_t node, uint16_t net);
        void recv(int i);

        virtual void run (XBeeFrame* frame);
        virtual void wake_up();

};

}

#endif // __XBEE_COMM_H__
