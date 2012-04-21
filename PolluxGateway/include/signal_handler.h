#include <signal.h>

extern "C" {
void sigint_handler(int c) {
    beagle::Leds::disable_leds();
    beagle::UART::disable_uart2();
    printf("Exiting...\n");
    exit(0);
}

/*** Sets up a handler for signals (INT/TERM/HUP) so the applications resets properly
 * all the beaglebone's features.
 */
void setup_signal() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);
    sigaction(SIGHUP, &sigIntHandler, NULL);
}
}
