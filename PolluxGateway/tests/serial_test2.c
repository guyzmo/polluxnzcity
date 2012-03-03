#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

int            n;
int            socket;
int            fd;
int            max_fd;
fd_set         input;
struct timeval timeout;

/* Initialize the input set */
FD_ZERO(&input);
FD_SET(fd, &input);
FD_SET(sock, &input);

max_fd = (sock > fd ? sock : fd) + 1;

/* Initialize the timeout structure */
timeout.tv_sec  = 10;
timeout.tv_usec = 0;

/* Do the select */
n = select(max_fd, &input, NULL, NULL, &timeout);

/* See if there was an error */
if (n < 0)
	perror("select failed");
else if (n == 0)
	puts("TIMEOUT");
	else
{
	/* We have input */
	if (FD_ISSET(fd, &input))
		process_fd();
	if (FD_ISSET(sock, &input))
		process_socket();
}
