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
 * Main routine
 */

//#define TIMING 50
//#define VERBOSE 
//#define API_ESCAPED_MODE // if API MODE is 2, useful for use with software flow control

#include <version.h>
#include <cli_parser.h>
#include <signal_handler.h>

#include <beaglebone/beaglebone.h>

#include <xbee/toolbox.h>
#include <xbee/xbee_result.h>
#include <xbee/xbee_communicator.h>

#include <pollux/pollux_observer.h>
#include <pollux/pollux_calibrator.h>

#include <string>
#include <iostream>

#include <inttypes.h>

int main(int argc, char* argv[]) {

    try {
        Cli_parser cli_args(argc, argv);

        // get -v
        if (cli_args.has("-v") || cli_args.has("--version")) {
#ifdef VERBOSE
            std::cout<<"Pollux'NZ City Gateway module -- "<<POLLUX_VERSION<<" -- +verbose"<<std::endl;
#else
            std::cout<<"Pollux'NZ City Gateway module -- "<<POLLUX_VERSION<<std::endl;
#endif
        }

        // get -h
        if (cli_args.has("-h") || cli_args.has("--help")) {
#ifdef VERBOSE
            std::cout<<"Pollux'NZ City Gateway module -- "<<POLLUX_VERSION<<" -- +verbose"<<std::endl;
#else
            std::cout<<"Pollux'NZ City Gateway module -- "<<POLLUX_VERSION<<std::endl;
#endif
            std::cout<<"usage: "<<argv[0]<<"[-h] [-p PATH]"<<std::endl\
                <<std::endl\
                <<"Pollux'NZ City Gateway module"<<std::endl\
                <<std::endl\
                <<"optional arguments:"<<std::endl\
                <<"	-c, --conf PATH		Path to the configuration directory (default: /etc/pollux)"<<std::endl\
                <<"	-e, --ext PATH		Path to the plugin base directory (default: /usr/lib/pollux)"<<std::endl\
                <<"	-s, --superuser 	Does not fork, and escalates down privileges"<<std::endl\
                <<"	-v, --version		Returns version"<<std::endl\
                <<"	-h, --help		This help screen"<<std::endl\
                <<std::endl;
            ::exit(0);
        }

        // get -s
        if (cli_args.has("-s") || cli_args.has("--superuser"))
            configure_system_nofork();
        else
            configure_system();

        std::string path_conf= "/etc/pollux";
        std::string path_ext= "/usr/lib/pollux";

        // get -c
        if (cli_args.has("-c") && !cli_args.has("--conf"))
            path_conf = cli_args.get("-c");
        else if (!cli_args.has("-c") && cli_args.has("--conf"))
            path_conf = cli_args.get("--conf");
        else if (cli_args.has("-c") && cli_args.has("--conf")) {
            std::cerr<<"Can't have both -c or --conf. Please choose one. Exiting..."<<std::endl;
            ::exit(1);
        }

        // strip trailing '/' at end of path
        if (path_conf.length() > 0) {
            std::string::iterator it = path_conf.end() - 1;
            if (*it == '/')
                path_conf.erase(it);
        }
        
        // get -e
        if (cli_args.has("-e") && !cli_args.has("--ext"))
            path_ext = cli_args.get("-e");
        else if (!cli_args.has("-e") && cli_args.has("--ext"))
            path_ext = cli_args.get("--ext");
        else if (cli_args.has("-e") && cli_args.has("--ext")) {
            std::cerr<<"Can't have both -e or --ext. Please choose one. Exiting..."<<std::endl;
            ::exit(1);
        }
        
        // strip trailing '/' at end of path
        if (path_ext.length() > 0) {
            std::string::iterator it = path_ext.end() - 1;
            if (*it == '/')
                path_ext.erase(it);
        }

        pollux::Pollux_calibrator pconfig(path_conf, path_ext);

        try {
            pconfig.load_configuration();
            pconfig.load_datastores();
            pconfig.load_sensors();
            pconfig.load_geoloc();
        } catch (pollux::Pollux_config_exception pce) {
            std::cerr<<pce.what()<<std::endl;
            std::cerr<<"Can't load configuration, exiting..."<<std::endl;
            ::exit(1);
        }

        pollux::Pollux_observer s(pconfig);

        if (s.begin(B9600) >= 0) {
            for (;;)
                s.poll();
            return 0;
        }
    } catch (std::runtime_error e) {
        std::cerr<<e.what()<<std::endl;
    } catch (std::exception e) {
        std::cerr<<e.what()<<std::endl;
        std::cerr<<"uncaught exception"<<std::endl;
    } catch (...) {
        std::cerr<<"unbelievable exception"<<std::endl;
    }
    return 2;
}

