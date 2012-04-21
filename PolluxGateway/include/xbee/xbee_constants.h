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
 * Main routines
 */
#ifndef __XBEE_CONSTANTS_H__
#define __XBEE_CONSTANTS_H__

#include <inttypes.h>

// --------------------------------------------------------------- Constants

/** API frame fields */
#define FRM_DLM	0x7e		//frame delimiter

/** frame types	*/
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

/** errors */

#define ERR_WRONG_FRM_DLM     -1
#define ERR_NOT_IMPLEMENTED   -2
#define ERR_UNKWNOWN_FRM_ID   -4
#define ERR_INVALID_CHECKSUM  -8

/** Timing between each frame event */
#ifndef TIMING
#   define TIMING 10
#endif

namespace xbee {

/** default addresses */
const uint8_t COORDINATOR_ADDR[8] = {0,0,0,0,0,0,0,0};
const uint8_t BROADCAST_ADDR[8] = {0,0,0,0,0,0,0xff,0xff};
const uint16_t BROADCAST_NET = 0xFFFE;

}

#endif
