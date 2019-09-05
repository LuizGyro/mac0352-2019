#ifndef __UTIL_H__
#define __UTIL_H__

int getFiles(char *path, char *buffer, int max_size);

int openPort( int minPort, int maxPort, int *socketId);

int getName( char *input, char *output, int max_size);

#endif