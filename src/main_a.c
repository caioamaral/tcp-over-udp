/*
 * main_a.c
 *
 *  Created on: 3 Nov 2014
 *      Author: caio
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "transport/frame.h"
#include <netinet/tcp.h>   // Declaracoes do tcp header

void error(const char *);

char host[] = "localhost";
char myPort[] = "5524";

/*
struct _tcp_header {
	 unsigned int source_port:16;
	 unsigned int destination_port:16;
	 unsigned int sequence_num:32;
	 unsigned int ack_num:32;
	 unsigned int header_size:4;
	 unsigned int reserved:6;
	 unsigned int flag_urg:1;
	 unsigned int flag_ack:1;
	 unsigned int flag_psh:1;
	 unsigned int flag_rst:1;
	 unsigned int flag_syn:1;
	 unsigned int flag_end:1;
	 unsigned int rcv_window:16;
};

tcp_header build_header(int destPort, int seqNum, int ackNum, int recvWindow,
		int urg, int ack, int psh, int rst, int syn, int end){
	tcp_header pkt;
	pkt.source_port = atoi(myPort);
	pkt.destination_port = destPort;
	pkt.header_size = 0;
	pkt.rcv_window = 0;
	pkt.ack_num = ackNum;
	pkt.flag_ack = ack;
	pkt.flag_urg = urg;
	pkt.flag_psh = psh;
	pkt.flag_rst = rst;
	pkt.flag_syn = syn;
	pkt.flag_end = end;
	return pkt;
}

*/
int main(int argc, char *argv[])
{
   int sock, n;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *hp;
   char buffer[256];

   /*
   if (argc != 2) { printf("Usage: server port\n");
                    exit(1);
   }
   */
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(host);
   if (hp==0) error("Unknown host");

   bcopy((char *)hp->h_addr,
        (char *)&server.sin_addr,
         hp->h_length);
   server.sin_port = htons(atoi(myPort));
   length=sizeof(struct sockaddr_in);
   printf("Please enter the message: ");
   bzero(buffer,256);
   fgets(buffer,255,stdin);
   //TcpPacket msg = assemblePkt(5524, 1, 0, 222, 0, 0, 0, 0, 1, 0, "hahahahahhahaha");

   n=sendto(sock,buffer,
            strlen(buffer),0,(const struct sockaddr *)&server,length);
   if (n < 0) error("Sendto");
   n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
   if (n < 0) error("recvfrom");
   write(1,"Got an ack: ",12);
   write(1,buffer,n);
   close(sock);
   return 0;
}


