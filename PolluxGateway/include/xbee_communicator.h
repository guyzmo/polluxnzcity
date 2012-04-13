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

#include <pollux_toolbox.h>
#include <beagle_serial.h>

// --------------------------------------------------------------- Constants

/* API frame fields */
#define FRM_DLM	0x7e		//frame delimiter

/* frame types	*/
#define API_AT_CMD     0x08    //send at command
#define API_RM_CMD     0x17    //remote command request
#define API_XMIT_REQ   0x10    //transmit request	
#define API_ADDR_REQ   0x11    //explicit addressing transmit request	

#define AT_CMD_RESP    0x88    //AT Command Response
#define RM_CMD_RESP    0x97    //Remote Command Response
#define MODEM_STATUS   0x8A    //Modem Status
#define TX_STATUS      0x8B    //ZigBee Transmit Status
#define RX_PACKET      0x90    //ZigBee Receive Packet (AO=0)
#define RX_PACKET_IND  0x91    //ZigBee Explicit Rx Indicator (AO=1)
#define IO_SMPL_IND_RX 0x92    //ZigBee IO Data Sample Rx Indicator
#define SENSOR_RD_IND  0x94    //XBee Sensor Read Indicator (AO=0)
#define NODE_INDICATOR 0x95    //Node Identification Indicator (AO=0)

#define RX_PACKET_LEN  12      // 12 bytes of frame specific data between header and checksum

/* errors */

#define ERR_WRONG_FRM_DLM     -1
#define ERR_NOT_IMPLEMENTED   -2
#define ERR_UNKWNOWN_FRM_ID   -4
#define ERR_INVALID_CHECKSUM  -8

/* Timing between each frame event */
#ifndef TIMING
#   define TIMING 10
#endif

/* default addresses */
const uint8_t COORDINATOR_ADDR[8] = {0,0,0,0,0,0,0,0};
const uint8_t BROADCAST_ADDR[8] = {0,0,0,0,0,0,0xff,0xff};
const uint16_t BROADCAST_NET = 0xFFFE;

// --------------------------------------------------------------- Xbee Union Types

typedef union two_bytes {
    uint16_t i16;
    struct bytes {
        uint8_t lsb;
        uint8_t msb;
    } i8;
} uint8_16_u;

typedef struct xbee_frame {
    uint8_16_u length;
    uint8_t api_id;
    union frm_content {
        struct at {
            uint8_t source_addr[8];
            uint8_16_u network_addr;
            uint8_t frame_id;
            char command[2];
            uint8_t status;
            char values[100];
        } at;
        struct tx {
            uint8_t frame_id;
            uint8_16_u network_addr;
            uint8_t retries;
            uint8_t delivery_status;
            uint8_t discovery_status;
        } tx;
        struct rx {
            uint8_t source_addr[8];
            uint8_16_u network_addr;
            uint8_t options;
            uint8_t payload[100];
        } rx;
        struct modem_status {
            uint8_t status;
        } ms;
        uint8_t unknown[111];
    } content;
    uint16_t checksum;
} XBeeFrame;

// --------------------------------------------------------------- XBEE Lib

class Xbee_communicator : public Serial {

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

        int begin ();

        int read(bool no_esc=false);
        ssize_t write(uint8_t i);

        void send(char* payload);
        void send(char* payload, uint8_t* node, uint16_t net);
        void send(char* payload, uint64_t node, uint16_t net);
        void recv(int i);

        virtual void run (XBeeFrame* frame);
        virtual void wake_up();

};

#endif // __XBEE_COMM_H__
