#define _SVID_SOURCE
/* For futimens */
#define _GNU_SOURCE
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

/* We only want the eventX entries in /dev/input */
int evmatch(const struct dirent *entry) {
    return (strncmp("event",entry->d_name,5) == 0);
}

int main(void) {
    struct dirent **inputs; /* Find all of the eventX entries in /dev/input */
    int numfds = scandir("/dev/input", &inputs, evmatch, 0);
    if (numfds < 0) {
        perror("Cannot scan /dev/input");
        exit(1);
    }

    /* Open our status file */
    int evfd = open("/var/spool/input", O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, 0666);
    if (evfd == -1 ) {
        perror("Cannot open /var/spool/input: ");
        exit(1);
    }

    close(0);
    close(1);
    close(2);

    fd_set rfds;
    FD_ZERO(&rfds);
    int nfds = 0, inputfd[__FD_SETSIZE], i;
    char inputname[__FD_SETSIZE][FILENAME_MAX];

    /* Open all of the event files and add them to our select list */
    for (i=0; i<numfds; i++) {
        sprintf(inputname[i],"/dev/input/%s",inputs[i]->d_name);
        inputfd[i] = open(inputname[i], O_RDONLY);
        nfds = max(nfds, inputfd[i]);
        FD_SET(inputfd[i], &rfds);
    }
    nfds++;

    for (;;) {
        /* Reload file descriptors to watch */
        fd_set srfds = rfds;

        /* Wait for input */
        int retval = select(nfds, &srfds, NULL, NULL, NULL);

        if (retval > 0) {
            /* Drain all input */
            for (i=0; i<numfds; i++) {
                if (FD_ISSET(inputfd[i],&srfds)) {
                    char buf[2048];
                    while (read(inputfd[i], buf, 2048) == 2048) {}
                }
            }

            /* Update the status file */
            futimens(evfd, NULL);
            struct timespec req, rem;

            /* Sleep - we don't need high time resolution on status file */
            req.tv_sec = 10;
            req.tv_nsec = 0;
            nanosleep(&req, &rem);
        }
    }
}
