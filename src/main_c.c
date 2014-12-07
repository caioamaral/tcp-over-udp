#include "transport/transport.h"




void str_ser(int sockfd); // transmitting and receiving function

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

    // binds the socket to all available interfaces
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        printf("error in binding\n");
        perror("socket error");
        exit(1);
    }

    // receive and ACK
    str_ser(sockfd);

    close(sockfd);
    exit(0);
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
                if ((n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&processa_addr, len)) == -1)
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