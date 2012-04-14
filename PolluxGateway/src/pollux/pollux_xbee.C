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
#include <pollux/pollux_xbee.h>

extern "C" {
void sigint_handler(int c) {
    beagle::Leds::disable_leds();
    beagle::UART::disable_uart2();
    printf("Exiting...\n");
    exit(0);
}
}

using namespace pollux;

void Pollux_observer::setup_signal() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}
Pollux_observer::Pollux_observer(Pollux_configurator& conf) : Xbee_communicator(conf.get_config_option(std::string("tty_port")), 
                                                                atoi(conf.get_config_option(std::string("wud_sleep_time")).c_str())), config(conf) { 
    beagle::UART::enable_uart2();
    beagle::Leds::enable_leds();
    beagle::Leds::set_status_led();
    this->setup_signal();
    meas_idx=0;
}
Pollux_observer::~Pollux_observer() {
    beagle::Leds::disable_leds();
}

void Pollux_observer::get_next_measure(xbee::Xbee_result& frame) {
    char* buffer = config.next_measure(frame.get_node_address_as_long());

    debug_printf("buffer to send: %02X, %02X, %02X\n", buffer[0], buffer[1], buffer[2]);
    if (buffer[0] == 0x0)
        std::cout<<"   -> skipping measure step"<<std::endl;
    else {
        printf("   -> sending measure to node %llx\n", frame.get_node_address_as_long());
        send(buffer, frame.get_node_address(), frame.get_network());
    }
    delete(buffer);
}

void Pollux_observer::wake_up() {
    const char high[] = { 0x5, 0x0 };
    const char low[] = { 0x4, 0x0 };
    long long unsigned int module = config.next_module();

    std::cout<<"waking up module: "<<std::hex<<module<<std::endl;

    this->send_remote_atcmd(module, 0xFFFF, "D0", high);
    msleep(50);
    this->send_remote_atcmd(module, 0xFFFF, "D0", low);
}

void Pollux_observer::run (xbee::XBeeFrame* frame) {

#ifdef VERBOSE
    Xbee_communicator::run(frame); // print frame details
#endif
    xbee::Xbee_result payload(frame);

    switch (frame->api_id) {
        case AT_CMD_RESP:
            //beagle::Leds::set_rgb_led(beagle::Leds::GREEN);

            printf("[AT] Command: '%s' ; Values: '%X'\n", frame->content.at.command, frame->content.at.values);

            msleep(50);
            //beagle::Leds::reset_rgb_led(beagle::Leds::GREEN);
            break;
        case RX_PACKET:
            beagle::Leds::set_rgb_led(beagle::Leds::BLUE);

            payload.print();

            switch (payload.get_i2c_command()) {
                case CMD_MEAS:
                    std::cout<<"   <- recv measure"<<std::endl;
                    config.store_measure(payload);
                    get_next_measure(payload);
                    break;
                case CMD_INIT:
                    std::cout<<"   <- recv wake up"<<std::endl;
                    get_next_measure(payload);
                    break;
                case CMD_HALT:
                    std::cout<<"   <- recv sleep down"<<std::endl;
                    config.push_data(payload.get_node_address_as_long());
                    break;
                case '*':
                    // just a comment ;)
                    break;
                default:
                    printf("Unknown Command: %d\n", payload.fmt_i2c_command());
            }

            msleep(10);
            beagle::Leds::reset_rgb_led(beagle::Leds::BLUE);
            break;
        case TX_STATUS:
            if (frame->content.tx.delivery_status != 0x00) {
                std::cerr<<"Error sending frame: "<<std::endl;
                switch (frame->content.tx.delivery_status) {
                    case 0x00: std::cerr<<"Success"<<std::endl; break;
                    case 0x02: std::cerr<<"CCA Failure"<<std::endl; break;
                    case 0x15: std::cerr<<"Invalid destination endpoint"<<std::endl; break;
                    case 0x21: std::cerr<<"Network ACK Failure"<<std::endl; break;
                    case 0x22: std::cerr<<"Not Joined to Network"<<std::endl; break;
                    case 0x23: std::cerr<<"Self-addressed"<<std::endl; break;
                    case 0x24: std::cerr<<"Address Not Found"<<std::endl; break;
                    case 0x25: std::cerr<<"Route Not Found"<<std::endl; break;
                }
            }
        default:
            beagle::Leds::set_rgb_led(beagle::Leds::RED);
            msleep(10);
            printf("Incoming frame (%02X) that's not useful.\n", frame->api_id);
            beagle::Leds::reset_rgb_led(beagle::Leds::RED);
            break;
    }
}
