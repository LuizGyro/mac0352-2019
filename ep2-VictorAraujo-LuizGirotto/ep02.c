#include "worker.h"
#include "leader.h"
#include "immortal.h"
#include "file.h"
#include "util.h"
#include "lln.h"
#include "llip.h"

#define OUTFILE "result.txt"

int
main( int argc, char **argv) {
    if (argc > 1) {
        pid_t worker_pid;
        if ((worker_pid = fork()) == 0) {
            worker();
        }
        else {
            int number_files = splitFiles( argv[1]);
            char **out_files = malloc( number_files * sizeof( char *));
            immortal( number_files, out_files);
            fkmerge( OUTFILE, out_files, number_files);
            for (int i = 0; i < number_files; i++) {
                free( out_files[i]);
            }
            free( out_files);
        }
    }
    else {
        worker();
    }
    return 0;
}
