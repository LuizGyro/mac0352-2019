#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#define LISTENQ 1
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <linux/netdevice.h>
#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include "file.h"


/* ///////////////////////////////////////////// */
/* ////////////////// Globals ////////////////// */
/* ///////////////////////////////////////////// */

/* Process type definers */
bool is_immortal = false;
bool is_leader = false;
bool is_worker = false;
bool is_listener = false;

/* Process type-specific variables */
bool is_working = false; /* Only for workers */
bool want_election = false; /* Only for workers */

int *work_array; /* Only for immortal. 0 means that the file still needs to be ordered. */

/* General control variables */
bool all_work_done = false; /* Specifies when all work is done, and process should be terminated */

/* ///////////////////////////////////////////// */
/* ///////////////// Functions ///////////////// */
/* ///////////////////////////////////////////// */


/* ///////////////////////////////////////////// */
/* /////////////////// Main //////////////////// */
/* ///////////////////////////////////////////// */

int main( int argc, char **argv) {
    pid_t childpid;

    if (argc > 1) {
        /* Is IMMORTAL-Generic */
        is_immortal = true;

        /* Start of program: pre-process data, make self leader  */
        /* begin working (IMMORTAL-Worker) while waiting for     */
        /* other computers to join in. */
        int n_files = splitFiles(argv[1]);



        if ((childpid = fork()) == 0) {
            /* Is LEADER-Generic */
            is_leader = true;
            exit(0);
        }
        else {
            /* Is IMMORTAL-Generic */

            exit(0);
        }




    }
    else {
        /* Is, for now, WORKER-Listener */
        is_worker = true;
    }

    return 0;
}
