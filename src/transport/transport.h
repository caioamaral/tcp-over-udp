/*
 * transport.h
 *
 *  Created on: 2 Nov 2014
 *      Author: caio, daniel, adeline
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"


#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350

#define A_PORT 6533
#define B_PORT 6245
#define C_PORT 7337

#define DATALEN 65
#define BUFSIZE 6000000

#define FIFO_MAX_SIZE 50
#define PAYLOAD_SIZE 1024
#define NO_ACK_RATE 0 // 10%
#define ERROR_RATE_B 10
#define WRONG_ACK_RATE 0 // 10%
#define TRUE 1
#define FALSE 0
#define TIMEOUT 1
#define PACKET_SIZE 1032
#define TRANSMISSION_INTERVAL 10



struct data_frame
{
    uint32_t num; // numero de sequencia
    uint32_t len; // Tamanho pacote
    char data[PAYLOAD_SIZE]; // the packet data
};

struct ack_frame
{
    uint32_t num; // numero de sequencia
    uint32_t len; // Tamanho pacote
};