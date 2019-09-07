#ifndef __UTIL_H__
#define __UTIL_H__

/* Esta função armazena em buffer o nome de todos os arquivos
** encontrados no diretorio path. O tamanho máximo de buffer é
** dado por max_size, a função retorna 1 se algum erro ocorrer
** e 0 caso contrário.
*/
int getFiles(char *path, char *buffer, int max_size);

/* Esta função ira percorrer as portas entre minPort e maxPort,
** e ira tentar criar um socket escutando tal porta. O socket sera
** identificado por socketId e a porta escolhida sera retornada
** pela função. No caso de erro a função ira retornar 1.
*/
int openPort( int minPort, int maxPort, int *socketId);

/* Esta função ira guardar em output input[5 ... n], sendo max_size
** o tamanho máximo de output, em caso de erro 1 sera retornado.
*/
int getName( char *input, char *output, int max_size);

#endif