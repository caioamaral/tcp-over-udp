#include "transport/transport.h"
#include <pthread.h>



void str_ser(int sockfd); // transmitting and receiving function
void *transmitter(void *param);
void *receiver(void *param);


// Threads
pthread_t thread_receiver, thread_transmitter;

pthread_mutex_t fifo_counter = PTHREAD_MUTEX_INITIALIZER;

// Global
int stop_transmission;
int created_fifo;
int fifo[2];
int packets_on_fifo;
struct sockaddr_in processc_addr;
int sockfd;


int main(int argc, char *argv[])
{
    created_fifo = 0;
    stop_transmission = 0;
    int result;
    packets_on_fifo = 0;

    if(pthread_create(&thread_transmitter, NULL, transmitter, NULL) != 0){
        printf("error ao criar thread de recepcao");
        exit(0);
    }
    if(pthread_create(&thread_receiver, NULL, receiver, NULL) != 0){
        printf("error ao criar thread de recepcao");
        exit(0);
    }

    result = pipe(fifo);
    if (result < 0){
        perror("pipe");
        exit(1);
    }


    pthread_join(thread_receiver, NULL);
    pthread_join(thread_transmitter,   NULL);
    pthread_mutex_destroy(&fifo_counter);
    // receive and ACK
    //str_ser(insockfd);

    return 0;
}

void *receiver(void *param){
    // receive via socket from process c, and write on the fifo.

    struct sockaddr_in my_addr, addr;
    char packet_c[1032];
    int result;    
    socklen_t len = sizeof(struct sockaddr_in);


/*
    fifo = open("fifo_queue", O_WRONLY);
    if (fifo == -1) {
        printf("Cannot open fifo");
        exit(0);
    }

    printf("fifo aberta \n");
*/
    //create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("error in socket\n");
        exit(0);
    }

    my_addr.sin_family = AF_INET; 
    my_addr.sin_port = htons(B_PORT); 
    my_addr.sin_addr.s_addr = INADDR_ANY; 

    bzero(&(my_addr.sin_zero), 8);

    // binds the socket to all available interfaces
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        printf("error in binding\n");
        perror("socket error");
        exit(1);
    }

    //str_ser(insockfd);
    struct data_frame packet;
    int i = 0;
    int n;
    while(stop_transmission == 0){
        if ((n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            printf("Error when receiving\n");
            exit(1);
        }

        // recebeu nada
        else if (n == 0)
        {
            printf("Nothing received\n");
        }

        // chegou pacote
        else
        {
            printf(BLUE "** Arrived a packet from process A. Seq.Num: %i Pkt.Len %i\n" RESET, packet.num, packet.len);
            // Funcao Perde mensagem
            if ((rand() % 100) < ERROR_RATE_B)
            {
                // perdeu pacote na entrada
                printf(RED "**** Lost Packet on Entrance: Ops!.\n" RESET);
            }
            else 
            {   
                pthread_mutex_lock(&fifo_counter);
                if(packets_on_fifo < FIFO_MAX_SIZE){
                    result = write (fifo[1], &packet,sizeof(packet));
                    packets_on_fifo++; 
                    printf("**** Added packet to FIFO. Packets there: %i\n", packets_on_fifo);         
                }
                else{
                    printf(RED "**** Couldnt Put packet on Fifo: FULL.\n" RESET);  
                }
                pthread_mutex_unlock(&fifo_counter);
                
                if (result < 1){
                    perror ("write: ");
                    exit (2);
                }
            }
        }    
    }
    close(fifo);
    close(sockfd);
    return 1;
}


void *transmitter(void *param) {
    // Local variables
    int res;
    struct data_frame packet;
    int n = 0;
    int random;
    int packet_on_buffer_sent = 1;
    socklen_t len = sizeof(struct sockaddr_in);

    processc_addr.sin_family = AF_INET; 
    processc_addr.sin_port = htons(C_PORT); 
    processc_addr.sin_addr.s_addr = INADDR_ANY; 

    
    while(stop_transmission == 0) {
        // Expl:
        // Le_a_Fifo()
        // if (tinha um pacote la){
        //      envia esse pacote pro processc
        // }
        // sleep(INTERVALO);

        pthread_mutex_lock(&fifo_counter);
        if(packets_on_fifo > 0){
            n = read(fifo[0], &packet, sizeof(packet));
            printf(GREEN "--- Lido da Fifo: Seq.Num: %i Pkt.Len: %i\n" RESET, packet.num, packet.len);
            packet_on_buffer_sent = 0;
            if (n < 1) {
                printf(RED "ERROR Fifo empty\n" RESET);
            } 
            packets_on_fifo--;
        }
        pthread_mutex_unlock(&fifo_counter);
        //
        
        if(packet_on_buffer_sent == 0){
            printf(GREEN "-------- Sending packet to ProcessC\n" RESET);
            if ((n = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&processc_addr, len)) < 0)
            {
                printf(RED "ERROR: Nao pode enviar pacote para processoC!\n" RESET);
                exit(1);
            }
            printf(GREEN "Sent.\n" RESET);
            packet_on_buffer_sent = 1;
        }
        
        random = rand() % 50;
        usleep(TRANSMISSION_INTERVAL*random);
        
    }
    printf("Encerrando o Transmissor...\n");
    return 1;
}


// transmitting and receiving function
void str_ser(int sockfd)
{   
    FILE *fp;
    char buf[BUFSIZE];
    int end = 0, n = 0;
    long lseek = 0;
    struct ack_frame ack;
    struct data_frame packet;

    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    printf("Start receiving...\n");

    srand(time(NULL)); // seed for random number
    uint32_t prev_pkt_seq = 1;

    while(!end)
    {
        /*************** RECEIVE MESSAGE ***************/
        // if error in receiving
        if ((n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            printf("Error when receiving\n");
            exit(1);
        }

        // if nothing received
        else if (n == 0)
        {
            printf("Nothing received\n");
        }

        // if something received
        else
        {
            // random number 0-99
            // ACK lost
            // send ACK
            if ((rand() % 100) > NO_ACK_RATE)
            {
                // tell sender what to expect next
                if (packet.num == 0)
                    ack.num = packet.len + 1;
                else
                    ack.num = packet.num + packet.len;
                ack.len = packet.len;

                // random number 0-99
                // ACK damaged
                // damage ACK by toggling ACK
                /*
                if ((rand() % 100) < WRONG_ACK_RATE)
                {
                    if (ack.num == 0)
                        ack.num = 1;
                    else
                        ack.num = 0;
                    printf("ACK damaged! ");
                }
                */

                /*************** SEND ACK ***************/
                if ((n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1)
                {
                    printf("ACK send error!\n");
                    exit(1);
                }
                printf("Received packet seq.num: %i data size: %i\n", packet.num, packet.len);
                printf("%i %i as ACK sent\n", ack.num, ack.len);
            }
            // does not send ACK
            else
                printf("ACK lost!\n");

            // only save packet if it is not a duplicate
            if (packet.num != prev_pkt_seq)
            {   
                // if the last bit of the received string is the EoF
                if (packet.data[packet.len-1] == '\0')
                {
                    end = 1;
                    packet.len--;
                }

                // copy this packet
                memcpy((buf+lseek), packet.data, packet.len);
                lseek += packet.len;
            }

            // record down previous packet sequence
            prev_pkt_seq = packet.num;
        }
    }

    if ((fp = fopen ("received.jpg", "wt")) == NULL)
    {
        printf("File doesn't exit\n");
        exit(0);
    }

    fwrite (buf, 1, lseek, fp); //write data into file
    fclose(fp);
    printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)lseek);
}