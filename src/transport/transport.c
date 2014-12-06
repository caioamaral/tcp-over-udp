/*
 * transport.c
 *
 *  Created on: 2 Nov 2014
 *      Author: caio, daniel, adeline
 */


#include "transport.h"


#define MSG_SIZE 1024


int create_socket(){
	int sock = 0;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0){
		printf("\nCan't open socket fd");
		exit(0);
	}
	return sock;
}

void bind_my_socket(int mysocket, int myport){
	struct sockaddr_in server;

	bzero((char *) &server, sizeof(server));
  	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = htonl(INADDR_ANY);
  	server.sin_port = htons(myport);

    if (bind(mysocket, (struct sockaddr *) &server, sizeof(server)) < 0) {
    	printf("ERROR on binding\n");
    	exit(0);
    }
}

struct sockaddr_in get_server_addr(char * dest_name, int dest_port){
	struct sockaddr_in serv_addr;
	struct hostent *server;

	server = gethostbyname(dest_name);
	
	if(server == NULL){
		perror(strcat("Can't find host named: %s", dest_name));
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
	 server->h_length);
	serv_addr.sin_port = htons(dest_port);

	return serv_addr;
}

void connect_to_other_socket(int mysocket, struct sockaddr_in server){	
	if(connect(mysocket,(struct sockaddr *) &server,sizeof(server)) < 0){
		printf("Can't connect to other socket port");
		exit(0);
	}
}

void listen_my_socket(int socket, int max_con){
	listen(socket, max_con);
}


int receive_message(int sockfd, char *message)
{
  int len;

  bzero(message, MSG_SIZE);
  
  len = read(sockfd, message, MSG_SIZE);

  if (len < 0)
    printf("Impossivel ler socket");
    		
  return len;
}

