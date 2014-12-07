// -----------------------------------------------------//
//  Trabalho Redes 2014.2                               //
//  Caio Amaral, Daniel Valenti e Adeline Feriaux-Rubin //

#include "transport/transport.h"

float envia_para_espera(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen, socklen_t *len_recvfrom);
void calcula_tempo_transmissao(struct timeval *out, struct timeval *in);


struct sockaddr_in processb_addr, my_addr, processc_addr;


int main(int argc, char **argv)
{
    int sockfd;
    float ti, rt;
    long len;

    struct hostent *sh;   // hostname
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
    printf("Nome do Host destino: %s\n", sh->h_name);

    // Criando Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd <0)
    {
        printf(RED "ERROR: Nao foi possivel criar socket\n" RESET);
        exit(1);
    }

    processb_addr.sin_family = AF_INET;                                               
    processb_addr.sin_port = htons(B_PORT);
    processb_addr.sin_addr.s_addr = INADDR_ANY; 
    
    my_addr.sin_family = AF_INET;                                               
    my_addr.sin_port = htons(A_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY; 

    // binds the socket to all available interfaces
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        printf(RED "ERROR in binding\n" RESET);
        perror("Socket error");
        exit(1);
    }

    memcpy(&(processb_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    bzero(&(processb_addr.sin_zero), 8);

    if((fp = fopen ("in.txt","r+t")) == NULL)
    {
        printf(RED "ERROR: Arquivo nao existe\n" RESET);
        exit(0);
    }

    // Envia pelo protocolo para-espera
    ti = envia_para_espera(fp, sockfd, &len, (struct sockaddr *)&processb_addr, 
        sizeof(struct sockaddr_in), &len_recvfrom);

    rt = ((len-1) / (float)ti); // calcula a media de taxa de transmissao
    printf(MAGENTA "\nTempo(ms) : %.3f, Enviados(bytes): %d\nData rate: %f (Kbytes/s)\n" RESET, ti, (int)len-1, rt);

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
    printf(BLUE "Tamanho a enviar: %d bytes\n" RESET, (int)file_size);
    printf("Payload do pacote: %d bytes\n", PAYLOAD_SIZE);

    // Aloca a memoria para conter todo o arquivo.
    buf = (char *) malloc(file_size + 1);
    if (buf == NULL)
       exit (2);

    // Copia arquivo pro buffer
    fread(buf, 1, file_size, fp);

    // O arquivo inteiro eh carregado no buffer
    buf[file_size] ='\0'; // anexa o ultimo byte de terminacao
    gettimeofday(&sendt, NULL); // pega o tempo atual

    while(acked_bytes <= file_size)
    {
        /*************** SEND MESSAGE ***************/
        if (last_msg_acked) // se a msg anterior ja recebeu o ack, pode transmitir mais uma msg
        {
            // montando o pacote
            if ((file_size - acked_bytes + 1) <= PAYLOAD_SIZE) // if(eh o ultimo pacote)
                packet.len = file_size - acked_bytes + 1; // monta com o resto dos bytes
            else // ainda nao esta no ultimo frame, envia msg com payload cheio
                packet.len = PAYLOAD_SIZE;

            packet.num = next_seq_num;
            memcpy(packet.data, (buf + acked_bytes), packet.len);

            // envio da mensagem
            gettimeofday(&sendTime, NULL);
            if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
            {
                printf(RED "Erro ao enviar a mensagem.\n" RESET);
                exit(1);
            }
            else{
                printf(YELLOW"Enviado novo pacote. Num.Seq: %i Len: %i\n"RESET, packet.num, packet.len);
            }

            // atualizando o proximo numero da sequencia
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
        // if(nada_recebido)
        if ((n = recvfrom(sockfd, &ack, sizeof(ack), MSG_DONTWAIT, (struct sockaddr *) &processc_addr, len_recvfrom)) == -1)
        {
            // monitora o tempo sem receber nada
            gettimeofday(&curTime, NULL);
            // if timeout
            if (curTime.tv_sec - sendTime.tv_sec > TIMEOUT)
            {
                // retransmit
                printf(RED "Timeout! Reenviando.\n" RESET);
                /*************** RESEND MESSAGE ***************/
                gettimeofday(&sendTime, NULL);
                if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
                {
                    printf(RED "ERROR: Erro ao enviar mensagem!\n" RESET); // send the data
                    exit(1);
                }
            }
        }

        // Senao: Recebeu um ack
        else
        {
            printf("\nUm novo ack foi recebido: Num: %d\n", ack.num);
            
            // se o ACK estiver incorreto
            // if(nao eh o ack esperado OU payload tamanho diferente que o descrito)
            if (ack.num != next_seq_num || ack.len != packet.len)
            {
                printf(RED "ACK Incorreto. Reenviando pacote.\n" RESET);
                printf("(Esperado -> Num: %i Len:%i , mas recebido -> Num: %i Len: %i)\n", next_seq_num, packet.len, ack.num, ack.len);

                /*************** RESEND MESSAGE ***************/
                gettimeofday(&sendTime, NULL);
                if((n = sendto(sockfd, &packet, sizeof(packet), 0, addr, addrlen)) == -1)
                {
                    printf(RED"ERROR: Send error!\n"RESET); // send the data
                    exit(1);
                }
            }
            else // Senao: ACK esta correto!
            {
                printf(GREEN "Ack Correto! Prosseguindo com o envio....\n" RESET);
                last_msg_acked = TRUE;
            }
        }
    }

    gettimeofday(&recvt, NULL); // Pega o tempo de fim de transmissao
    *len= acked_bytes;
    calcula_tempo_transmissao(&recvt, &sendt);
    time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;

    return(time_inv);
}


void calcula_tempo_transmissao(struct  timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) <0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }

    out->tv_sec -= in->tv_sec;
}