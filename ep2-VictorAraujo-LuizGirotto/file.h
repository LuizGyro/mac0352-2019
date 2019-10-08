#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX 5

int getMinInd ( long long int *arr, int n);

void fkmerge( char *out_name, char **files, int n);

void orderFile( char *in_name, char *out_name);

FILE *newFile( FILE *fd, int file_number);

int splitFiles( char *file_name);

#endif
