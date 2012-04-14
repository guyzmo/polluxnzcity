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
#ifndef __XBEE_TYPES_H__
#define __XBEE_TYPES_H__

#include <inttypes.h>

// --------------------------------------------------------------- Xbee Union Types

namespace xbee {

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

}

#endif
