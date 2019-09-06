#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <stdbool.h>
#include "util.h"

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

#define MINPORT 2000
#define MAXPORT 8000

int main (int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
    * e o outro que vai ser o socket específico de cada conexão */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct */
    struct sockaddr_in servaddr;
    /* Retorno da função fork para saber quem é o processo filho e quem
    * é o processo pai */
    pid_t childpid;
    /* Armazena linhas recebidas do cliente */
    char recvline[MAXLINE + 1];
    /* Armazena o tamanho da string lida do cliente */
    ssize_t n;

    if (argc != 2) {
        fprintf( stderr, "Uso: %s <Porta>\n", argv[0]);
        fprintf( stderr, "Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit( 1);
    }

    /* Criação de um socket. Eh como se fosse um descritor de arquivo. Eh
    * possivel fazer operacoes como read, write e close. Neste
    * caso o socket criado eh um socket IPv4 (por causa do AF_INET),
    * que vai usar TCP (por causa do SOCK_STREAM), já que o FTP
    * funciona sobre TCP, e será usado para uma aplicação convencional sobre
    * a Internet (por causa do número 0) */
    if ((listenfd = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
        perror( "socket :(\n");
        exit( 2);
    }

    /* Agora é necessário informar os endereços associados a este
    * socket. É necessário informar o endereço / interface e a porta,
    * pois mais adiante o socket ficará esperando conexões nesta porta
    * e neste(s) endereços. Para isso é necessário preencher a struct
    * servaddr. É necessário colocar lá o tipo de socket (No nosso
    * caso AF_INET porque é IPv4), em qual endereço / interface serão
    * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
    * qual a porta. Neste caso será a porta que foi passada como
    * argumento no shell (atoi(argv[1]))
    */
    bzero( &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind( listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror( "bind :(\n");
        exit( 3);
    }

    /* Como este código é o código de um servidor, o socket será um
    * socket passivo. Para isto é necessário chamar a função listen
    * que define que este é um socket de servidor que ficará esperando
    * por conexões nos endereços definidos na função bind. */
    if (listen( listenfd, LISTENQ) == -1) {
        perror( "listen :(\n");
        exit( 4);
    }

    printf( "[Servidor no ar. Aguardando conexoes na porta %s]\n",argv[1]);
    printf( "[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    /* O servidor no final das contas é um loop infinito de espera por
    * conexões e processamento de cada uma individualmente */
    for (;;) {
        /* O socket inicial que foi criado é o socket que vai aguardar
        * pela conexão na porta especificada. Mas pode ser que existam
        * diversos clientes conectando no servidor. Por isso deve-se
        * utilizar a função accept. Esta função vai retirar uma conexão
        * da fila de conexões que foram aceitas no socket listenfd e
        * vai criar um socket específico para esta conexão. O descritor
        * deste novo socket é o retorno da função accept. */
        if ((connfd = accept( listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
           perror( "accept :(\n");
           exit( 5);
        }

        /* Agora o servidor precisa tratar este cliente de forma
         * separada. Para isto é criado um processo filho usando a
         * função fork. O processo vai ser uma cópia deste. Depois da
         * função fork, os dois processos (pai e filho) estarão no mesmo
         * ponto do código, mas cada um terá um PID diferente. Assim é
         * possível diferenciar o que cada processo terá que fazer. O
         * filho tem que processar a requisição do cliente. O pai tem
         * que voltar no loop para continuar aceitando novas conexões */
        /* Se o retorno da função fork for zero, é porque está no
         * processo filho. */
        if ((childpid = fork()) == 0) {
           /**** PROCESSO FILHO ****/
           printf( "[Uma conexao aberta]\n");
           /* Já que está no processo filho, não precisa mais do socket
            * listenfd. Só o processo pai precisa deste socket. */
           close( listenfd);

           /* Agora pode ler do socket e escrever no socket. Isto tem
            * que ser feito em sincronia com o cliente. Não faz sentido
            * ler sem ter o que ler. Ou seja, neste caso está sendo
            * considerado que o cliente vai enviar algo para o servidor.
            * O servidor vai processar o que tiver sido enviado e vai
            * enviar uma resposta para o cliente (Que precisará estar
            * esperando por esta resposta)
            */

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 INÍCIO                        */
            /* ========================================================= */
            /* ========================================================= */
            /* TODO: É esta parte do código que terá que ser modificada
             * para que este servidor consiga interpretar comandos FTP   */

            bool can_pass = false;
            bool logged_in = false;
            int data_socket = -1;
            int data_stream;
            char name[MAXLINE + 1];
            char path[MAXLINE + 1];
            char buffer[MAXLINE + 1];
            char extra[MAXLINE + 1];

            write( connfd, "220 Service ready\r\n", 19 * sizeof( char));

            while ((n = read( connfd, recvline, MAXLINE)) > 0) {
                recvline[n] = 0;
                printf( "[Cliente conectado no processo filho %d enviou:] ", getpid());
                if ((fputs( recvline, stdout)) == EOF) {
                    perror( "fputs :( \n");
                    exit( 6);
                }
                fflush( stdout);

                if (!strncmp( recvline, "USER ", 5 * sizeof( char))) {
                    logged_in = false;
                    if (getName( recvline, name, MAXLINE + 1))
                        write( connfd, "500 Syntax error, command unrecognized. This may include errors such as command line too long.\r\n", 96 * sizeof( char));
                    else {
                        write( connfd, "331 User name okay, need password.\r\n", 36 * sizeof( char));
                        strncpy( path, "/home/", 10);
                        strncat( path, name, MAXLINE);
                        can_pass = true;
                    }
                }

                else if (!strncmp( recvline, "PASS ", 5 * sizeof( char))) {
                    if ( can_pass) {
                        if (chdir( path) == -1)
                            write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                        else {
                            write( connfd, "230 User logged in, proceed.\r\n", 30 * sizeof( char));
                            logged_in = true;
                        }
                    }
                    else
                        write( connfd, "503 Bad sequence of commands.\r\n", 31 * sizeof( char));
                    can_pass = false;
                }

                else if (!strncmp( recvline, "QUIT\r\n", 6 * sizeof( char))) {
                    write( connfd, "221 Service closing control connection.\nLogged out if appropriate.\r\n", 68 * sizeof( char));
                    break;
                }

                else if (!strncmp( recvline, "DELE ", 5 * sizeof( char))) {
                    if (!logged_in)
                        write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                    else if (getName( recvline, name, MAXLINE + 1))
                        write( connfd, "500 Syntax error, command unrecognized. This may include errors such as command line too long.\r\n", 96 * sizeof( char));
                    else {
                        if (remove( name) == -1)
                            write( connfd, "550 Requested action not taken. File unavailable (e.g., file not found, no access).\r\n", 85 * sizeof( char));
                       else
                            write( connfd, "250 Requested file action okay, completed.\r\n", 44 * sizeof( char));
                    }
                }

                else if (!strncmp( recvline, "LIST\r\n", 6 * sizeof( char)) ||
                !strncmp( recvline, "LIST ", 5 * sizeof( char))) {

                    if (!logged_in)
                        write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                    else {
                        if (getName( recvline, name, MAXLINE + 1))
                            strncpy( extra, ".\0", MAXLINE);
                        else {
                            strncpy( extra, path, MAXLINE);
                            strcat( extra, "/");
                            strcat( extra, name);
                        }
                        if (getFiles( extra, buffer, MAXLINE))
                            write( connfd, "501 Syntax error in parameters or arguments.\r\n", 46 * sizeof( char));
                        else {
                            if (data_socket == -1) {
                                write( connfd, "450 Requested file action not taken.\r\n", 38 * sizeof( char));
                            }
                            else {
                                write( connfd, "150 File status okay; about to open data connection.\r\n", 54 * sizeof( char));
                                data_stream = accept( data_socket, NULL, NULL);
                                write( data_stream, buffer, strlen( buffer));
                                close( data_stream);
                                close( data_socket);
                                write( connfd, "226 Closing data connection.Requested file action successful\r\n", 62 * sizeof( char));
                                data_socket = -1;
                            }
                        }
                    }
                }

                //STOR <SP> <pathname> <CRLF>
                else if (!strncmp( recvline, "STOR ", 5 * sizeof( char))) {
                    if (!logged_in)
                        write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                    else {
                        if (getName( recvline, name, MAXLINE + 1))
                            write( connfd, "501 Syntax error in parameters or arguments.\r\n", 46 * sizeof( char));
                        else {
                            if (data_socket == -1)
                                write( connfd, "503 Bad sequence of commands.\r\n", 31 * sizeof( char));
                            else {
                                FILE *fl;
                                char small_buffer[1];
                                if ((fl = fopen( name, "w")) == NULL) {
                                    printf("Problem opening the file %s\n", strerror(errno));
                                    write( connfd, "451 Requested action aborted: local error in processing.\r\n", 58 * sizeof( char));
                                }
                                else {
                                    write( connfd, "150 File status okay; about to open data connection.\r\n", 54 * sizeof( char));
                                    /* Vamos ir lendo de byte em byte */
                                    data_stream = accept( data_socket, NULL, NULL);
                                    while ((read( data_stream, small_buffer, 1)) > 0) {
                                        fprintf(fl, "%c", small_buffer[0]);
                                    }
                                    close( data_stream);
                                    close( data_socket);
                                    fclose(fl);
                                    write( connfd, "226 Closing data connection.Requested file action successful.\r\n", 63 * sizeof( char));
                                    data_socket = -1;
                                }
                            }
                        }
                    }
                }
                //RETR <SP> <pathname> <CRLF>
                else if (!strncmp( recvline, "RETR ", 5 * sizeof( char))) {
                    if (!logged_in)
                        write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                    else {
                        if (getName( recvline, name, MAXLINE + 1))
                            write( connfd, "501 Syntax error in parameters or arguments.\r\n", 46 * sizeof( char));
                        else {
                            if (data_socket == -1)
                                write( connfd, "503 Bad sequence of commands.\r\n", 31 * sizeof( char));
                            else {
                                FILE *fl;
                                char *big_buffer;
                                struct stat *file_mdata = malloc( sizeof( struct stat));
                                if ((fl = fopen( name, "r")) == NULL) {
                                    printf("Problem opening the file %s\n", strerror(errno));
                                    write( connfd, "550 file not found.\r\n", 21 * sizeof( char));
                                }
                                else {
                                    if (stat( name, file_mdata) == -1) {
                                        printf("Problem getting the stats %s\n", strerror(errno));
                                        write( connfd, "451 Requested action aborted: local error in processing.\r\n", 58 * sizeof( char));
                                    }
                                    write( connfd, "150 File status okay; about to open data connection.\r\n", 54 * sizeof( char));
                                    big_buffer = malloc( file_mdata->st_size);
                                    fread( big_buffer, 1, file_mdata->st_size, fl);
                                    data_stream = accept( data_socket, NULL, NULL);
                                    write( data_stream, big_buffer, strlen( big_buffer));
                                    close( data_stream);
                                    close( data_socket);
                                    free(big_buffer);
                                    fclose(fl);
                                    write( connfd, "226 Closing data connection.Requested file action successful.\r\n", 63 * sizeof( char));
                                    data_socket = -1;
                                }
                            }
                        }
                    }

                }

                else if (!strncmp( recvline, "PASV\r\n", 6 * sizeof( char))) {
                    if (!logged_in)
                        write( connfd, "530 Not logged in.\r\n", 20 * sizeof( char));
                    else {
                        int data_port = openPort( MINPORT, MAXPORT, &data_socket);
                        if (data_port == 1)
                            write( connfd, "420 Not abble to open socket.\r\n", 31 * sizeof( char));
                        else {
                            sprintf( buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n",
                            127 & 0xff, 0 & 0xff, 0 & 0xff,
                            1 & 0xff, data_port & 0xff, (data_port >> 8) & 0xff);
                            write( connfd, buffer, strlen(buffer) * sizeof( char));
                        }
                    }
                }

                else if (!strncmp( recvline, "TYPE ", 5 * sizeof( char))) {
                    if (strlen( recvline) < 6)
                        write( connfd, "501 Syntax error in parameters or arguments.\r\n", 46 * sizeof( char));
                    else if (recvline[5] != 'I')
                        write( connfd, "504 Command not implemented for that parameter.\r\n", 49 * sizeof( char));
                    else {
                        write( connfd, "200 Command okay.\r\n", 19 * sizeof( char));
                    }
                }

                else if (!strncmp( recvline, "SYST\r\n", 6 * sizeof( char))) {
                    write( connfd, "502 Command not implemented.\r\n", 30 * sizeof( char));
                }
                else {
                    write( connfd, "502 Command not implemented.\r\n", 30 * sizeof( char));
                }
            }
            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 FIM                           */
            /* ========================================================= */
            /* ========================================================= */

            /* Após ter feito toda a troca de informação com o cliente,
             * pode finalizar o processo filho */
            printf( "[Uma conexao fechada]\n");
            exit( 0);
        }
        /**** PROCESSO PAI ****/
        /* Se for o pai, a única coisa a ser feita é fechar o socket
        * connfd (ele é o socket do cliente específico que será tratado
        * pelo processo filho) */
        close( connfd);
    }
    exit( 0);
}
