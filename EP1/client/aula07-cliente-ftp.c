/* Código simples de um cliente FTP que se conecta no servidor, loga
 * com usuário e senha e depois roda o comando 'quit'.
 * Não é o código ideal (deveria ter returns ou exits após os erros
 * das funções por exemplo) mas é suficiente para exemplificar o
 * conceito.
 * O código também não espera o servidor enviar todos os dados (cada
 * read precisaria estar num loop mas com um temporizador pois se após
 * tanto tempo não vier resposta, é porque terminou de enviar dados e
 * o cliente pode voltar a enviar comandos)
 * 
 * RFC do FTP: http://www.faqs.org/rfcs/rfc959.html
 *
 * Prof. Daniel Batista em 23/08/2011. Modificado em cima do cliente
 * do http da aula 05
 *
 * Bugs? Tente consertar primeiro! Depois me envie email :) batista@ime.usp.br
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 10000

int main(int argc, char **argv) {
   int sockfd, n;
   char recvline[MAXLINE + 1];
   char sndline[MAXLINE + 1];
   char scanline[MAXLINE + 1];
   struct sockaddr_in servaddr;
   
   if (argc != 2) {
      fprintf(stderr,"usage: %s <IPaddress>\n",argv[0]);
      exit(1);
   }

   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      fprintf(stderr,"socket error :( \n");
   
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(21);  /* FTP padrao roda na 21 */

   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
      fprintf(stderr,"inet_pton error for %s :(\n", argv[1]);
   
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
      fprintf(stderr,"connect error :(\n");

   /* Funcionamento do FTP:
      1-) Cliente conecta no servidor
      2-) Servidor envia um conjunto de dados e pára no pedido de usuário
      3-) Usuário envia o username
      4-) Servidor pede a senha
      5-) Usuário envia a senha
      6-) Servidor envia um conjunto de dados e pára no prompt de comandos
      7-) Cliente roda QUIT
    */

   /* 2-) */
   if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF)
         fprintf(stderr,"fputs error :(\n");
   }
   else
      fprintf(stderr,"read error :(\n");

   /* 3-) */
   strcpy(sndline,"USER ");
   printf("Digite seu usuario (anonymous se o servidor aceitar): ");
   scanf("%s",scanline);
   strcat(sndline,scanline);
   strcat(sndline,"\r\n");
 
   if (write(sockfd,sndline,strlen(sndline)) < 0)
      fprintf(stderr,"write error :(\n"); 
   
   /* 4-) */
   if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF)
         fprintf(stderr,"fputs error :(\n");
   }
   else
      fprintf(stderr,"read error :(\n"); 

   /* 5-) */
   printf("Digite sua senha (usuario@dominio se o usuario foi anonymous): ");
   strcpy(sndline,"PASS ");
   scanf("%s",scanline);
   strcat(sndline,scanline);
   strcat(sndline,"\r\n");

   if (write(sockfd,sndline,strlen(sndline)) < 0)
      fprintf(stderr,"write error :(\n"); 
   
   /* 6-) */ 
   if ((n = read(sockfd, recvline,MAXLINE)) > 0) {
         recvline[n] = 0;
         if (fputs(recvline, stdout) == EOF)
            fprintf(stderr,"fputs error :(\n");
         fprintf(stderr,"%d",n);
   }
   if (n<0)
      fprintf(stderr,"read error :(\n");
   
   /* 7-) */
   printf("Enviando um QUIT para o servidor...\n\n");
   strcpy(sndline,"QUIT\r\n");

   if (write(sockfd,sndline,strlen(sndline)) < 0)
      fprintf(stderr,"write error :(\n"); 
 
   exit(0);
}
