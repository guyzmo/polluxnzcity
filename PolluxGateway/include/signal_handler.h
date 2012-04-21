#include <beaglebone/beaglebone.h>
#include <beaglebone/beagle_serial.h>
#include <signal.h>

#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>


extern "C" {
int pid;

void sig_root_handler(int c) {
    beagle::Leds::disable_leds();
    beagle::UART::disable_uart2();
    kill(pid, SIGTERM);
    printf("Exiting...\n");
    exit(0);
}

void configure_system() {
    if ((pid = fork()) != 0) {
        // sets up the signal in Parent process
        beagle::UART::enable_uart2();
        beagle::Leds::enable_leds();
        beagle::Leds::set_status_led();

        // set up signal
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = sig_root_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);
        sigaction(SIGTERM, &sigIntHandler, NULL);
        sigaction(SIGHUP, &sigIntHandler, NULL);
        
        while (true) {
            int status;
            pid_t done = ::wait(&status);
            if (done == -1) {
                if (errno == ECHILD) break; // no more child processes
            } else {
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                    printf("pid %d failed\n", done);
                    ::exit(1);
                }
            }
        }
        beagle::Leds::disable_leds();
        beagle::UART::disable_uart2();
        ::exit(1);
    }

    // get privileges down in the child process
    
    // find user and group
    struct passwd * pwd = getpwnam("pollux");
    struct group * grp = getgrnam("dialout");

    // not included : error checking

    uid_t uid = pwd->pw_uid;
    gid_t gid = grp->gr_gid;

    setgid(gid);
    setuid(uid);
}

}

