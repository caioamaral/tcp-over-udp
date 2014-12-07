// -----------------------------------------------------//
//  Trabalho Redes 2014.2                               //
//  Caio Amaral, Daniel Valenti e Adeline Feriaux-Rubin //

#include "transport/transport.h"
#include <pthread.h>


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



    // Cria o socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("error in socket\n");
        exit(0);
    }

    if(pthread_create(&thread_transmitter, NULL, transmitter, NULL) != 0){
        printf(RED "Error ao criar thread de Transmissao" RESET);
        exit(0);
    }
    if(pthread_create(&thread_receiver, NULL, receiver, NULL) != 0){
        printf(RED"Error ao criar thread de Recepcao"RESET);
        exit(0);
    }

    result = pipe(fifo);
    if (result < 0){
        perror("FIFO ERROR");
        exit(1);
    }

    pthread_join(thread_receiver, NULL);
    pthread_join(thread_transmitter,   NULL);
    pthread_mutex_destroy(&fifo_counter);

    return 0;
}

void *receiver(void *param){
    // recebe via socket do processo A, e escreve na fifo.

    printf(YELLOW"Processo B iniciado.."RESET);

    struct sockaddr_in my_addr, addr;
    char packet_c[1032];
    int result;    
    socklen_t len = sizeof(struct sockaddr_in);

    my_addr.sin_family = AF_INET; 
    my_addr.sin_port = htons(B_PORT); 
    my_addr.sin_addr.s_addr = INADDR_ANY; 

    bzero(&(my_addr.sin_zero), 8);

    // bind no socket na porta do processo B
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        printf("error in binding\n");
        perror("socket error");
        exit(1);
    }

    struct data_frame packet;
    int i = 0;
    int n;
    while(stop_transmission == 0){ // Enquanto nao pediu pra encerrar a transmissao.
        // Recebe no socket
        if ((n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            printf("Error when receiving\n");
            exit(1);
        }

        // recebeu nada
        else if (n == 0)
        {
            printf("Nada recebido\n");
        }

        // chegou pacote
        else
        {
            printf(BLUE "\n** Chegou um novo pacote do processo A. Seq.Num: %i Pkt.Len %i\n" RESET, packet.num, packet.len);
            // Funcao Perde mensagem
            if ((rand() % 100) < ERROR_RATE_B)
            {
                // perdeu pacote na entrada (propositalmente)
                printf(RED "###### Lost a Packet on Entrance: Ops!.#####\n" RESET);
            }
            else 
            {   
                pthread_mutex_lock(&fifo_counter);
                if(packets_on_fifo < FIFO_MAX_SIZE){
                    result = write (fifo[1], &packet,sizeof(packet));
                    packets_on_fifo++; 
                    printf("+++ Pacote adicionado na FIFO. Pacotes na fila: %i\n", packets_on_fifo);         
                }
                else{
                    printf(RED "**** Nao pode colocar pacote na Fifo: FULL.\n" RESET);  
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
            packets_on_fifo--;
            printf("--- Pacote Removido da Fifo Fifo: Seq.Num: %i Pkt.Len: %i. Pacotes na Fifo %i\n", packet.num, packet.len, packets_on_fifo);
            packet_on_buffer_sent = 0;
            if (n < 1) {
                printf(RED "ERROR Fifo empty\n" RESET);
            } 
            
        }
        pthread_mutex_unlock(&fifo_counter);
        
        if(packet_on_buffer_sent == 0){
            if ((n = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&processc_addr, len)) < 0)
            {
                printf(RED "ERROR: Nao pode enviar pacote para processoC!\n" RESET);
                exit(1);
            }
            printf(GREEN "Pacote enviado para o processo C\n" RESET);
            packet_on_buffer_sent = 1;
        }
        random = rand() % 50;
        usleep(TRANSMISSION_INTERVAL*random);
        
    }
    printf("Encerrando o Transmissor...\n");
    return 1;
}