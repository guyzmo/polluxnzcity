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

#include <xbee_communicator.h>

void Xbee_communicator::xmit_req(uint8_t* addr64, uint16_t network, uint8_t nbytes, uint8_t* data, uint8_t frameid, uint8_t bradius, uint8_t options) {
    int checksum;
    uint8_t len;

    char* byte_ptr = NULL;

    debug_print("Sending frame: '");
    debug_print_hex(FRM_DLM);
    Serial::write(FRM_DLM);                                        // 1. SEND FRAME DELIMITER
    // data's length + frm type (1) + frm id (1) + addr (8) + network (4) + bradius (1) + options (1)
    len = nbytes + 14;                 
    this->write((uint8_t)(len>>8));                                // 2. SEND LENGTH
    this->write((uint8_t)(len&0xFF));

    this->write(API_XMIT_REQ);          checksum = API_XMIT_REQ;   // 3. SEND FRAME API Identifier (transmission request)
    this->write(frameid);               checksum += frameid;       // 4. SEND FRAME Unique ID
    for (int8_t i=0;i<8;++i) {
        this->write(*(addr64+i));       checksum += *(addr64+i);   // 5. SEND 64bit ADDRESS OF REMOTE MODULE
    }
    byte_ptr = (char*)&network;
    for (int8_t i=1;i>=0;--i){
        this->write(*(byte_ptr+i));     checksum += *(byte_ptr+i); // 6. SEND PAN ID
    }
    this->write(bradius);               checksum += bradius;       // 7. BROADCAST
    this->write(options);               checksum += options;       // 8. OPTIONS

    // 9. SEND PAYLOAD
    for (uint8_t i=0;i<nbytes;++i) {
        this->write(data[i]);           checksum += data[i];
    }

    // 10. SEND CHECKSUM
    checksum = 0xff - (checksum);
    this->write(checksum);
    debug_println("'");
}

void Xbee_communicator::send_atcmd(const char* at_command, const char* param_value) {
    debug_print("send_at_cmd(");debug_print(at_command);debug_print(")\n");

    uint16_t len = 0;
    int checksum;

    debug_print("Sending frame: '");
    debug_print_hex(FRM_DLM);
    Serial::write(FRM_DLM);                                         // 1. SEND FRAME DELIMITER

    // 1B for frm id + 1B for api id + size of the command (usually 2B) + size of the param (usually 0B)
    len = strlen(at_command) + strlen(param_value)+2;
    this->write((uint8_t)(len>>8));                                 // 2. SEND FRAME LENGTH
    this->write((uint8_t)(len&0xFF));

    this->write(API_AT_CMD);            checksum = API_AT_CMD;      // 3. SEND FRAME API Identifier (transmission request)
    this->write(frm_id);                checksum += frm_id;        // 4. SEND FRAME Unique ID

    ++frm_id; // increment frame id for next frame

    for (uint8_t i=0;i<strlen(at_command);++i) {
        this->write(at_command[i]);     checksum += at_command[i];  // 5. SEND AT COMMAND
    }

    for (uint8_t i=0;i<strlen(param_value);++i) {
        this->write(param_value[i]);    checksum += param_value[i]; // 6. SEND PARAMETERS
    }

    checksum = 0xff - (checksum);                                   // 10. SEND CHECKSUM
    this->write(checksum);
    debug_println("'");
}

void Xbee_communicator::send_remote_atcmd(uint8_t* addr64, uint16_t network, const char* at_command, const char* param_value) {
    debug_print("send_remote_at_cmd(");debug_print(at_command);debug_print(")\n");

    uint16_t len = 0;
    int checksum;
    char* byte_ptr = NULL;

    debug_print("Sending frame: '");
    debug_print_hex(FRM_DLM);
    Serial::write(FRM_DLM);                                                 // 1. SEND FRAME DELIMITER

    // 1B for frm id + 1B for api id + 1B for option + 8 bytes for addr + 2 bytes for pan + size of the command (usually 2B) + size of the param (usually 0B)
    len = strlen(at_command) + strlen(param_value)+13;
    this->write((uint8_t)(len>>8));                                         // 2. SEND FRAME LENGTH
    this->write((uint8_t)(len&0xFF));

    this->write(API_RM_CMD);            checksum = API_RM_CMD;              // 3. SEND FRAME API Identifier (transmission request)
    this->write(frm_id);                checksum += frm_id;                 // 4. SEND FRAME Unique ID

    ++frm_id; // increment frame id for next frame

    for (int8_t i=0;i<8;++i) {
        this->write(*(addr64+i));       checksum += *(addr64+i);            // 5. SEND 64bit ADDRESS OF REMOTE MODULE
    }
    byte_ptr = (char*)&network;
    for (int8_t i=1;i>=0;--i){
        this->write(*(byte_ptr+i));     checksum += *(byte_ptr+i);          // 6. SEND PAN ID
    }

    this->write(0x02);                  checksum += 0x02;                   // 7. SEND OPTION (0x02 == commit changes without sending AC command)

    for (uint8_t i=0;i<strlen(at_command);++i) {
        this->write(at_command[i]);     checksum += at_command[i];  // 5. SEND AT COMMAND
    }

    for (uint8_t i=0;i<strlen(param_value);++i) {
        this->write(param_value[i]);    checksum += param_value[i]; // 6. SEND PARAMETERS
    }

    checksum = 0xff - (checksum);                                           // 10. SEND CHECKSUM
    this->write(checksum);
    debug_println("'");
}

int Xbee_communicator::rcpt_frame(XBeeFrame* frame) {
    uint8_t b;
    frame->checksum = 0;
    //1. GET FRAME DELIMITER
    debug_print("[FRM_DLM]");
    if (this->read(true) != FRM_DLM)
        return ERR_WRONG_FRM_DLM;

    //2. GET CONTENT LENGTH
    debug_print("[LEN]");
    frame->length.i8.msb = this->read();
    frame->length.i8.lsb = this->read();

    //3. GET COMMAND
    debug_print("[FRM_ID]");
    frame->api_id = this->read();
    switch (frame->api_id) {
        case AT_CMD_RESP:               
            frame->api_id = AT_CMD_RESP;                          frame->checksum += AT_CMD_RESP;
            frame->content.at.frame_id = this->read();            frame->checksum += frame->content.at.frame_id;
            frame->content.at.command[0] = this->read();          frame->checksum += frame->content.at.command[0];
            frame->content.at.command[1] = this->read();          frame->checksum += frame->content.at.command[1];
            frame->content.at.status = this->read();              frame->checksum += frame->content.at.status;
            for (uint8_t i=0;i<frame->length.i16-5;++i) {
                frame->content.at.values[i] = this->read();       frame->checksum += frame->content.at.values[i];
            }
            break;

        case RM_CMD_RESP:
            frame->api_id = RM_CMD_RESP;                          frame->checksum += RM_CMD_RESP;
            frame->content.at.frame_id = this->read();            frame->checksum += frame->content.at.frame_id;

            for (uint8_t i=0;i<8;++i) {
                frame->content.at.source_addr[i] = this->read();  frame->checksum += frame->content.at.source_addr[i];
            }
            frame->content.at.network_addr.i8.msb = this->read(); frame->checksum += frame->content.at.network_addr.i8.msb;
            frame->content.at.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.at.network_addr.i8.lsb;
            frame->content.at.command[0] = this->read();          frame->checksum += frame->content.at.command[0];
            frame->content.at.command[1] = this->read();          frame->checksum += frame->content.at.command[1];
            frame->content.at.status = this->read();              frame->checksum += frame->content.at.status;
            for (uint8_t i=0;i<frame->length.i16-15;++i) {
                frame->content.at.values[i] = this->read();       frame->checksum += frame->content.at.values[i];
            }
            break;


        case TX_STATUS:
            frame->api_id = TX_STATUS;                            frame->checksum += TX_STATUS;
            frame->content.tx.frame_id = this->read();            frame->checksum += frame->content.tx.frame_id;
            frame->content.tx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.tx.network_addr.i8.msb;
            frame->content.tx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.tx.network_addr.i8.lsb;
            frame->content.tx.retries = this->read();             frame->checksum += frame->content.tx.retries;
            frame->content.tx.delivery_status = this->read();     frame->checksum += frame->content.tx.delivery_status;
            frame->content.tx.discovery_status = this->read();    frame->checksum += frame->content.tx.discovery_status;
            break;

        case RX_PACKET:
            frame->api_id = RX_PACKET;                            frame->checksum += RX_PACKET;
            for (uint8_t i=0;i<8;++i) {
                frame->content.rx.source_addr[i] = this->read();  frame->checksum += frame->content.rx.source_addr[i];
            }
            frame->content.rx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.msb;
            frame->content.rx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.lsb;
            frame->content.rx.options = this->read();             frame->checksum += frame->content.rx.options;
            for (uint8_t i=0;i<frame->length.i16-RX_PACKET_LEN;++i) {
                frame->content.rx.payload[i] = this->read();      frame->checksum += frame->content.rx.payload[i];
            }
            frame->content.rx.payload[frame->length.i16-RX_PACKET_LEN] = '\0';
            break;

        case NODE_INDICATOR:
            frame->api_id = NODE_INDICATOR;                       frame->checksum = NODE_INDICATOR;

            debug_print("[SRC_ADDR(8)]");
            for (uint8_t i=0;i<8;++i) {
                frame->content.rx.source_addr[i] = this->read();  frame->checksum += frame->content.rx.source_addr[i];
            }

            debug_print("[NET_ADDR(2)]");
            frame->content.rx.network_addr.i8.msb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.msb;
            frame->content.rx.network_addr.i8.lsb = this->read(); frame->checksum += frame->content.rx.network_addr.i8.lsb;

            debug_print("[OPTIONS]");
            frame->content.rx.options = this->read();             frame->checksum += frame->content.rx.options;

            debug_print("[NET_ADDR(2)]");
            b = this->read(); frame->checksum += b;
            b = this->read(); frame->checksum += b;

            debug_print("[SRC_ADDR(8)]");
            for (uint8_t i=0;i<8;++i) {
                b = this->read();  frame->checksum += b;
            }

            {
                uint8_t i=0;
                debug_print("[PAYLOAD]");
                b = -1;
                while (b != 0x0) {
                    b = this->read();      frame->checksum += b;
                    frame->content.rx.payload[i] = b;
                    ++i;
                }
                frame->content.rx.payload[i] = '\0';
            }
            debug_print("[PRT_ADDR(2)]");
            frame->checksum += this->read(); // parent address (2 bytes)
            frame->checksum += this->read();
            debug_print("[DEV_TYPE]");
            frame->checksum += this->read(); // device type (1 byte)
            debug_print("[SRC_ACT]");
            frame->checksum += this->read(); // source action (1 byte)
            debug_print("[PROFILE(2)]");
            frame->checksum += this->read(); // profile id (2 bytes)
            frame->checksum += this->read(); 
            debug_print("[MANUFACTURER(2)]");
            frame->checksum += this->read(); // manufacturer id (2bytes)
            frame->checksum += this->read();

            break;

        case MODEM_STATUS:
            frame->api_id = MODEM_STATUS;                       frame->checksum = MODEM_STATUS;

            debug_print("[CMD_DATA]");
            frame->content.ms.status = this->read();                   frame->checksum += frame->content.ms.status;

            break;

        default:
            for (uint8_t i = 0; i < min(frame->length.i16,100); ++i) {
                frame->content.unknown[i] = this->read(); }
            return ERR_UNKWNOWN_FRM_ID;
    }

    debug_print("[CHECKSUM(");
    debug_printc(0xFF-(frame->checksum&0xFF));
    debug_print(")]");
    //4. validate checksum
    if (0xFF-(frame->checksum&0xFF) != (uint16_t)this->read())
        return ERR_INVALID_CHECKSUM;

    return 1;
}

void Xbee_communicator::print_data(uint8_t* data, uint16_t len, int type) const {
    if (type == HEX) {
        for (uint16_t i=0;i<min(len,100);++i)
            printf("%02X", (uint8_t)data[i]);
    } else  {
        for (uint16_t i=0;i<min(len,100);++i)
            printf("%c", (uint8_t)data[i]);
    }
    printf("\n");
}

void Xbee_communicator::print_frame(XBeeFrame* frame) const {
    printf("-- Frame --\n");
    printf("length: "); printf("%d\n", frame->length.i16);
    printf("api_id: "); printf("%02X", frame->api_id);
    switch (frame->api_id) {
        case NODE_INDICATOR:
            printf(" ; NODE IDENTIFICATION INDICATOR\n");
            printf("src_ad: "); print_data(frame->content.rx.source_addr,8,HEX);
            printf("net_ad: "); printf("%02X\n", frame->content.rx.network_addr.i16);
            printf("recvopt: "); printf("%02X\n", frame->content.rx.options,HEX);
            printf("nodname: "); printf("%s\n", (char*)frame->content.rx.payload);
            break;
        case MODEM_STATUS:
            printf(" ; MODEM STATUS\n");
            printf("status: ");
            switch (frame->content.ms.status) {
                case 0: printf("Hardware reset\n"); break;
                case 1: printf("Watchdog timer reset\n"); break;
                case 2: printf("Associated\n"); break;
                case 3: printf("Disassociated\n");  break;
                case 4: printf("Synchronization Lost (Beacon-enabled only)\n"); break;
                case 5: printf("Coordinator realignment\n"); break;
                case 6: printf("Coordinator started\n"); break;
            }
            break;
        case AT_CMD_RESP:
            printf(" ; AT COMMAND RESPONSE\n");
            printf("frm_id: "); printf("%02X\n", frame->content.at.frame_id);
            printf("at_cmd: "); printf("%s\n", frame->content.at.command);
            printf("status: "); printf("%02X ", frame->content.at.status);
            switch (frame->content.at.status) {
                case 0: printf("OK\n"); break;
                case 1: printf("Error\n"); break;
                case 2: printf("Invalid command\n"); break;
                case 3: printf("Invalid Parameter\n");  break;
            }
            printf("values: "); printf("%02X\n", frame->content.at.values);
            break;

        case RM_CMD_RESP:
            printf(" ; REMOTE AT COMMAND RESPONSE\n");
            printf("frm_id: "); printf("%02X\n", frame->content.at.frame_id);
            printf("src_ad: "); print_data(frame->content.at.source_addr,8,HEX);
            printf("net_ad: "); printf("%02X\n", frame->content.at.network_addr.i16);
            printf("at_cmd: "); printf("%s\n", frame->content.at.command);
            printf("status: "); printf("%02X ", frame->content.at.status);
            switch (frame->content.at.status) {
                case 0: printf("OK\n"); break;
                case 1: printf("Error\n"); break;
                case 2: printf("Invalid command\n"); break;
                case 3: printf("Invalid Parameter\n");  break;
            }
            printf("values: "); printf("%02X\n", frame->content.at.values);

            break;

        case TX_STATUS:
            printf(" ; TX STATUS\n");
            printf("frm_id: "); printf("%02X\n", frame->content.tx.frame_id);
            printf("net_ad: "); printf("%02X\n", frame->content.tx.network_addr.i16);
            printf("retry : "); printf("%d\n", frame->content.tx.retries);
            printf("delivery: "); printf("%02X ", frame->content.tx.delivery_status);
            switch (frame->content.tx.delivery_status) {
                case 0x00: printf("Success\n"); break;
                case 0x02: printf("CCA Failure\n"); break;
                case 0x15: printf("Invalid destination endpoint\n"); break;
                case 0x21: printf("Network ACK Failure\n"); break;
                case 0x22: printf("Not Joined to Network\n"); break;
                case 0x23: printf("Self-addressed\n"); break;
                case 0x24: printf("Address Not Found\n"); break;
                case 0x25: printf("Route Not Found\n"); break;
            }
            printf("discos: "); printf("%02X ", frame->content.tx.discovery_status);
            switch(frame->content.tx.discovery_status) {
                case 0x00: printf("No Discovery Overhead\n"); break;
                case 0x01: printf("Address Discovery \n"); break;
                case 0x02: printf("Route Discovery\n"); break;
                case 0x03: printf("Address and Route Discovery\n"); break;
            }
            break;

        case RX_PACKET:
            printf(" ; RX PACKET\n");
            printf("src_ad: "); print_data(frame->content.rx.source_addr,8,HEX);
            printf("net_ad: "); printf("%02X\n", frame->content.rx.network_addr.i16);
            printf("recvopt: "); printf("%02X ", frame->content.rx.options);
            switch (frame->content.rx.options) {
                case 0x01: printf("Packet Acknowledged\n"); break;
                case 0x02: printf("Broadcast Packet\n"); break;
            }
            printf("payload: "); print_data(frame->content.rx.payload,frame->length.i16,DEC);
            break;

        default:
            printf(" /!\\ UNKNOWN PACKET TYPE /!\\\n");
            printf("content: "); print_data(frame->content.unknown,frame->length.i16, HEX);
    }
    printf("-- End of Frame --\n");
}

Xbee_communicator::Xbee_communicator(const std::string& port, int poll_wait) : Serial(port, poll_wait) {}

int Xbee_communicator::begin() {
    debug_print("Xbee_communicator.begin()\n");
    int ret = Serial::begin(B9600);
    if (ret <= 0)
        return ret;

    frm_id = 1;

    // discovery
    this->send_atcmd("ND", "");
    msleep(1000);
    this->send_atcmd("MY", "");
    msleep(1000);

    wake_up();

    return ret;
}

int Xbee_communicator::read(bool no_esc) {
    try {
        int c = Serial::read();
        msleep(TIMING);
        debug_print_hex(c);
        debug_print(" ");
#ifdef API_ESCAPED_MODE
        if (no_esc == false && c == 0x7D)
            return Serial::read()^0x20;
#endif
        return c;
    } catch (SerialException e) {
        e.print_msg();
    }
}

ssize_t Xbee_communicator::write(uint8_t i) {
    size_t s=0;

    msleep(TIMING);
    debug_print(" ");
    switch (i) {
#ifdef API_ESCAPED_MODE
        case 0x7E:
        case 0x7D:
        case 0x11:
        case 0x13:
            s+=Serial::write(0x7D);
            s+=Serial::write(0x20^i);
            return s;
#endif
        default:
            debug_print_hex(i);
            return s+Serial::write((uint8_t)i);
    }
}

void Xbee_communicator::send(char* data) {
    this->xmit_req(/* hardware addr */ (uint8_t*)COORDINATOR_ADDR,
                /* network bcast */ (uint16_t)BROADCAST_NET,
                /* data's length */ strlen(data), 
                /* data          */ (uint8_t*)data, 
                /* frame id      */ (uint8_t)frm_id++, 
                /* bcast radius  */ (uint8_t)8, 
                /* options       */ (uint8_t)8);
    if (frm_id = 0xFF)
        frm_id = 1;
}
void Xbee_communicator::send(char* data, uint8_t* addr=(uint8_t*)COORDINATOR_ADDR, uint16_t network=(uint16_t)BROADCAST_NET) {
    this->xmit_req(/* hardware addr */ (uint8_t*)addr,
                /* network bcast */ (uint16_t)network, 
                /* data's length */ strlen(data), 
                /* data          */ (uint8_t*)data, 
                /* frame id      */ (uint8_t)frm_id++, 
                /* bcast radius  */ (uint8_t)8, 
                /* options       */ (uint8_t)0);
    if (frm_id = 0xFF)
        frm_id = 1;
}

void Xbee_communicator::recv(int i) {
    int err=0;
    char c=0;
    XBeeFrame frame;

    if (i == 1)
        return wake_up();

    for (int i=0;i<111;++i)
        frame.content.unknown[i] = 0x0;

    debug_print("Recv frame: '");
    err = this->rcpt_frame(&frame);
    debug_println("'");
    switch (err) {
        case ERR_WRONG_FRM_DLM:
            printf("---- ERROR CODE: %d ; INVALID FRAME DELIMITER ----\n", err);
            break;
        case ERR_INVALID_CHECKSUM:
            printf("---- ERROR CODE: %d ; INVALID CHECKSUM ----\n", err);
            break;
        case ERR_UNKWNOWN_FRM_ID:
        case ERR_NOT_IMPLEMENTED:
            printf("---- ERROR CODE: %d\n", err);
            try {
                while ((c = this->read()) != (char)-1)
                    printf("%02X",c);
                printf("\n----\n");
            } catch (SerialException e) {
                printf("\n");
                e.print_msg();
            }
            break;
        default:
            run(&frame);
    }
}

void Xbee_communicator::run (XBeeFrame* frame) {
    this->print_frame(frame);
}

void Xbee_communicator::wake_up() {
    printf("Got wakeup...\n");
}

