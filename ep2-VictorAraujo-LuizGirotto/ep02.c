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
#include "ll.h"


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
celula *alive_list; /* Only for immortal. Is sent on end of heartbeat to Lider */
celula *working_list; /* Only for immortal. Is sent on end of heartbeat to Lider */

/* General control variables */
bool all_work_done = false; /* Specifies when all work is done, and process should be terminated */

/* ///////////////////////////////////////////// */
/* ///////////////// Functions ///////////////// */
/* ///////////////////////////////////////////// */


/* ///////////////////////////////////////////// */
/* /////////////////// Main //////////////////// */
/* ///////////////////////////////////////////// */

int main( int argc, char **argv) {
    int i;
    pid_t childpid;

    if (argc > 1) {
        /* Is IMMORTAL-Generic */
        is_immortal = true;

        /* Start of program: pre-process data, make self leader  */
        /* begin working (IMMORTAL-Worker) while waiting for     */
        /* other computers to join in. */
        int n_files = splitFiles(argv[1]);
        n_files++;
        work_array = malloc (n_files * sizeof(int));
        for (i = 0; i < n_files; i++) {
            work_array[i] = 0;
        }

        if ((childpid = fork()) == 0) {
            /* Is IMMORTAL-Listener */
            exit(0);
        }
        else {
            if ((childpid = fork()) == 0) {
                /* Is WORKER-Generic (but also IMMORTAL) */
                /* Code here should match beggining of regular worker code */
                /* (Except when starting as is_leader) */

                is_worker = true;
                if ((childpid = fork()) == 0) {
                    /* Is WORKER-Listener (but also IMMORTAL) */
                    exit(0);
                }
                else {
                    /* Is WORKER-Generic (but also IMMORTAL) */
                    /* No final do loop WORKER-Generic, se temos is_leader, */
                    /* mas estamos no processo do WORKER-Generic, criamos */
                    /* um novo processo LEADER-Generic, e tiramos o */
                    /* is_leader do WORKER-Generic */

                    is_leader = true;

                    while(!all_work_done) {
                        if (is_leader) {
                            if ((childpid = fork()) == 0) {
                                /* Is LEADER-Generic (but also IMMORTAL) */
                                is_worker = false;
                                if ((childpid = fork()) == 0) {
                                    /* Is LEADER-Listener (but also IMMORTAL) */
                                    exit(0);
                                }
                                else {
                                    /* Is LEADER-Generic (but also IMMORTAL) */
                                    /* LEADER-Generic loop goes here */
                                    exit(0);
                                }
                            }
                            is_leader = false;
                        }
                    }

                    exit(0);

                }
            }
            else {
                /* Is IMMORTAL-Generic */
                exit(0);
            }
        }




    }
    else {
        /* Is WORKER-Generic */
        is_worker = true;
    }

    return 0;
}
