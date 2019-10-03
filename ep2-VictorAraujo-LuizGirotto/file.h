#ifndef __FILE_H__
#define __FILE_H__

int getMinInd ( long long int *arr, int n);

void fkmerge( char *out_name, char **files, int n);

static int cmpll( const void *addr_1, const void *addr_2);

void orderFile( char *in_name, char *out_name);

FILE *newFile( FILE *fd, int file_number);

int splitFiles( char *file_name);

#endif
