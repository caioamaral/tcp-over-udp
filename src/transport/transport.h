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

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350

#define A_PORT 6235
#define B_PORT 6543
#define C_PORT 7831

#define DATALEN 65
#define BUFSIZE 6000000

#define PAYLOAD_SIZE 1024
#define NO_ACK_RATE 0 // 10%
#define WRONG_ACK_RATE 0 // 10%
#define TRUE 1
#define FALSE 0
#define TIMEOUT 1


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