// -----------------------------------------------------//
//  Trabalho Redes 2014.2                               //
//  Caio Amaral, Daniel Valenti e Adeline Feriaux-Rubin //

#include "transport/transport.h"


void receptor(int sockfd);

struct sockaddr_in processa_addr;



int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in my_addr;


    //create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("error in socket\n");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(C_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    processa_addr.sin_family = AF_INET;
    processa_addr.sin_port = htons(A_PORT); 
    processa_addr.sin_addr.s_addr = INADDR_ANY;

    bzero(&(my_addr.sin_zero), 8);

    // bind no socket
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        printf("error in binding\n");
        perror("socket error");
        exit(1);
    }

    // FUncao de recepcao e envio de ACK
    receptor(sockfd);

    close(sockfd);
    exit(0);
}

void receptor(int sockfd)
{   
    FILE *fp;
    char buf[BUFSIZE];
    int end = 0, n = 0;
    long lseek = 0;
    struct ack_frame ack;
    struct data_frame packet;


    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    printf(GREEN"Comecando a Recepcao...\n"RESET);

    srand(time(NULL)); // pega um numero randomico

    uint32_t prev_pkt_seq = 1;

    while(!end)
    {
        /*************** RECEIVE MESSAGE ***************/
        // if(erro na recepcao)
        if ((n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            printf(RED"Error when receiving\n"RESET);
            exit(1);
        }

        // if(nada_recebido)
        else if (n == 0)
        {
            printf("Nothing received...\n");
        }

        // if(recebeu_dados)
        else
        {
            // if(Numero random(0-99) > taxa_de_erro)
            // Enviar o Ack
            if ((rand() % 100) > NO_ACK_RATE)
            {
                // Montando o pacote ack, com o numero de sequencia esperado
                if (packet.num == 0)
                    ack.num = packet.len + 1;
                else
                    ack.num = packet.num + packet.len;
                ack.len = packet.len;


                /*************** SEND ACK ***************/
                if ((n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&processa_addr, len)) == -1)
                {
                    printf("ACK send error!\n");
                    exit(1);
                }
                printf(GREEN "\nPacote recebido! Seq.num: %i data size: %i\n" RESET, packet.num, packet.len);
                printf("%i %i em ACK enviado\n", ack.num, ack.len);
            }
            // nao envia ack propositalmente
            else
                printf(RED"ACK lost! Ops...\n"RESET);

            // salva o payload do patote se esse pacote nao eh duplicado
            if (packet.num != prev_pkt_seq)
            {   
                // se possue o terminador no ultimo byte do payload
                if (packet.data[packet.len-1] == '\0')
                {
                  // fim da transmissao
                    end = 1;
                    packet.len--;
                }

                // copia este pacote pra memoria
                memcpy((buf+lseek), packet.data, packet.len);
                lseek += packet.len;
            }

            // atualiza o numero de sequencia
            prev_pkt_seq = packet.num;
        }
    }
    // salva o arquivo com nome diferente
    if ((fp = fopen ("out.txt", "wt")) == NULL)
    {
        printf("File doesn't exit\n");
        exit(0);
    }

    fwrite (buf, 1, lseek, fp);
    fclose(fp);
    printf(MAGENTA"A file has been successfully received!\nThe total data received is %d bytes\n"RESET, (int)lseek);
}