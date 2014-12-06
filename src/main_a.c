#include "transport/transport.h"

float envia_para_espera(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen, socklen_t *len_recvfrom); // communication function
void tv_sub(struct timeval *out, struct timeval *in); //calculate the time interval between out and in

int main(int argc, char **argv)
{
    int sockfd;
    float ti, rt;
    long len;
    struct sockaddr_in ser_addr;
    char ** pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    FILE *fp;
    socklen_t len_recvfrom;

    if (argc != 2)
    {
        printf("Uso: processa <processc_host_name>\nEx: processa localhost\n");
        exit(0);
    }

    sh = gethostbyname(argv[1]);
    if (sh == NULL) 
    {
        printf("ERROR: Gethostbyname\n");
        exit(0);
    }

    addrs = (struct in_addr **)sh->h_addr_list;
    printf("Nome do Host destino: %s\n", sh->h_name); // print the remote host's information


    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // create the socket
    if (sockfd <0)
    {
        printf("ERROR: Nao foi possivel criar socket\n");
        exit(1);
    }

    ser_addr.sin_family = AF_INET;                                               
    ser_addr.sin_port = htons(B_PORT);
    
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);

    if((fp = fopen ("photo.jpg","r+t")) == NULL)
    {
        printf("ERROR: Arquivo nao existe\n");
        exit(0);
    }

    ti = envia_para_espera(fp, sockfd, &len, (struct sockaddr *)&ser_addr, 
        sizeof(struct sockaddr_in), &len_recvfrom);

    rt = ((len-1) / (float)ti); // caculate the average transmission rate
    printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len-1, rt);

    close(sockfd);
    fclose(fp);
    exit(0);
}


float envia_para_espera(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen, socklen_t *len_recvfrom)
{
    char *buf;
    uint32_t file_size, acked_bytes;
    struct data_frame packet;
    struct ack_frame ack;
    int n;
    float time_inv = 0.0;
    struct timeval sendt, recvt;
    struct timeval sendTime, curTime;
    acked_bytes = 0;

    int last_msg_acked = TRUE;
    uint32_t next_seq_num = 0;

    fseek(fp, 0, SEEK_END);
    file_size = ftell (fp);
    rewind(fp);
    printf("Tamanho a enviar: %d bytes\n", (int)file_size);
    printf("Payload do pacote: %d bytes\n", PAYLOAD_SIZE);

    // allocate memory to contain the whole file.
    buf = (char *) malloc(file_size + 1);
    if (buf == NULL)
       exit (2);

    // copy the file into the buffer.
    // read lsize data elements, each 1 byte
    fread(buf, 1, file_size, fp);

    // the whole file is loaded in the buffer
    buf[file_size] ='\0'; // append the end byte
    gettimeofday(&sendt, NULL); // get the current time
    while(acked_bytes <= file_size)
    {
        if (last_msg_acked) // se a msg anterior ja recebeu o ack, pode transmitir mais uma
        {
            // montando o pacote
            if ((file_size - acked_bytes + 1) <= PAYLOAD_SIZE) // aqui eh o ultimo frame
                packet.len = file_size - acked_bytes + 1;
            else // ainda nao esta no ultimo, envia msg com payload cheio
                packet.len = PAYLOAD_SIZE;

            packet.num = next_seq_num;
            memcpy(packet.data, (buf + acked_bytes), packet.len);

            // envio da mensagem
            gettimeofday(&sendTime, NULL);
            if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
            {
                printf("Erro ao enviar a mensagem.\n");
                exit(1);
            }

            // atualizando o numero de sequencia
            if(next_seq_num == 0){
                next_seq_num = packet.len + 1;
            }
            else{
                next_seq_num += packet.len;
            }
            acked_bytes += packet.len;
            last_msg_acked = FALSE;
        }

        /*************** RECEIVE ACK ***************/
        // MSG_DONTWAIT flag, non-blocking
        // receives nothing
        if ((n = recvfrom(sockfd, &ack, sizeof(ack), MSG_DONTWAIT, addr, len_recvfrom)) == -1)
        {
            // monitors how long nothing is received
            gettimeofday(&curTime, NULL);
            // if timeout
            if (curTime.tv_sec - sendTime.tv_sec > TIMEOUT)
            {
                // retransmit
                printf("Timeout! Resend this.\n");
                /*************** RESEND MESSAGE ***************/
                gettimeofday(&sendTime, NULL);
                if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
                {
                    printf("Send error!\n"); // send the data
                    exit(1);
                }
            }
        }

        // An ACK is received
        else
        {
            printf("ACK received. Num: %d\n", ack.num);
            // if what the server expects next is this one or server receives a different length
            // if ACK is incorrect
            if (ack.num != next_seq_num || ack.len != packet.len)
            {
                printf("Incorrect. Resend this. ");
                printf("(%i %i expected, but %i %i received)\n", next_seq_num, packet.len, ack.num, ack.len);

                /*************** RESEND MESSAGE ***************/
                gettimeofday(&sendTime, NULL);
                if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
                {
                    printf("Send error!\n"); // send the data
                    exit(1);
                }
            }
            // if ACK correct
            else
            {
                printf("Correct. Send next.\n");
                last_msg_acked = TRUE;
            }
        }
    }

    gettimeofday(&recvt, NULL);
    *len= acked_bytes; // get current time
    tv_sub(&recvt, &sendt); // get the whole trans time
    time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;

    return(time_inv);
}

//calculate the time interval between out and in
void tv_sub(struct  timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) <0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }

    out->tv_sec -= in->tv_sec;
}