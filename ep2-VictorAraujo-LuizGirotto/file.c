#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX 5

static int
cmpll( const void *addr_1, const void *addr_2) {
    long long int x1 = * (long long int *) addr_1;
    long long int x2 = * (long long int *) addr_2;

    if (x1 < x2) return -1;
    else if (x1 == x2) return 0;
    else return 1;
}

void
orderFile( char *in_name, char *out_name) {
    int i = 0;
    char *buffer;
    long long arr[MAX];
    FILE *in_fd = fopen( in_name, "r");
    if (in_fd == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return;
    }
    FILE *out_fd = fopen( out_name, "w");
    if (out_fd == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return;
    }

    buffer = malloc( 10000 * sizeof( char));
    if (buffer == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return;
    }

    while ( fgets( buffer, 10000, in_fd) != NULL)
        arr[i++] = atoll( buffer);

    qsort( arr, i, sizeof(long long int *), cmpll);

    for (int j = 0; j < i; j++)
        fprintf( out_fd, "%lld\n", arr[j]);

    free( buffer);
    fclose( in_fd);
    fclose( out_fd);
    return;
}

FILE *
newFile( FILE *fd, int file_number) {
    char *name = malloc( 200 * sizeof( char));
    char *number = malloc( 200 * sizeof( char));

    strncpy( name, "splitIn", 200 * sizeof( char));
    snprintf( number, 200 * sizeof( char), "%d", file_number);
    strncat( name, number, 200 * sizeof( char));
    strncat( name, ".txt", 200 * sizeof( char));

    fclose( fd);
    fd = fopen( name, "w");
    free( name);
    free( number);
    if (fd == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return NULL;
    }
    return fd;
}

int 
splitFiles( char *file_name) {
    FILE *fd1, *fd2;
    char *buffer;
    int lines, file_number;

    lines = 0;
    file_number = 0;

    buffer = malloc( 10000 * sizeof( char));
    if (buffer == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fd1 = fopen( file_name, "r");
    if (fd1 == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fd2 = fopen( "splitIn0.txt", "w");
    if (fd2 == NULL) {
        fprintf( stderr, "%s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    while ( fgets( buffer, 10000, fd1) != NULL) {
        if ( lines == MAX) {
            lines = 0;
            file_number++;
            fd2 = newFile( fd2, file_number);
        }
        fputs( buffer, fd2);
        lines++;
    }

    free( buffer);
    fclose( fd1);
    fclose( fd2);
    return file_number;
}

//Just for testing
void orderAllFiles( int n) {
    int i = 0;
    char *name_in = malloc( 200 * sizeof( char));
    char *name_out = malloc( 200 * sizeof( char));
    char *number = malloc( 200 * sizeof( char));
    while ( i < n) {
        strncpy( name_in, "splitIn", 200 * sizeof( char));
        snprintf( number, 200 * sizeof( char), "%d", i);
        strncat( name_in, number, 200 * sizeof( char));
        strncat( name_in, ".txt", 200 * sizeof( char));

        strncpy( name_out, "splitOut", 200 * sizeof( char));
        snprintf( number, 200 * sizeof( char), "%d", i);
        strncat( name_out, number, 200 * sizeof( char));
        strncat( name_out, ".txt", 200 * sizeof( char));
    
        orderFile( name_in, name_out);
        i++;
    }
    return;
}

int 
main( int argc, char **argv) {
    int n;
    if (argc < 1) {
        fprintf( stderr, "Not enough arguments\n\n");
        return EXIT_FAILURE;
    }
    n = splitFiles( argv[1]);
    orderAllFiles( n + 1);
    return 0;
}