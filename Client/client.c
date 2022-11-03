#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFSIZE 30

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len, char toSend[]);

int main(int argc, char *argv[]){
    int sd;
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    socklen_t from_address_len = sizeof(from_address);
    int portNumber;
    char serverIP[29];
    int rc = 0;

    

    if (argc < 3) {
        printf("Usage: %s <server ip> <port>\n", argv[0]);
        exit(1);
    }

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    portNumber = strtol(argv[2], NULL,10);
    strcpy(serverIP, argv[1]);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = inet_addr(serverIP);

    char buffer[18];
    char bufferIn[BUFFSIZE];
    char message[100];
    char leng[4];
    int maxLeng = 100;
    char *strPtr1;
    char *strPtr2;


    int sequenceNumber = 0;
    printf("TEST MESSAGE #1: How are you today? I'm fine thanks for asking\n");
    printf("TEST MESSAGE #2: How are you today? I'm fine thanks for asking!\n");


    printf("Enter a message\n");
    char *ptr = fgets(message, maxLeng, stdin);
    int length = strlen(message)-1;

    int networkLength = ntohl(length);

    rc = sendto(sd, &networkLength, sizeof(int), MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));
    
    int val = 0;
    while(val == 0){
        val = sendData(sd, server_address, from_address, from_address_len, message);
    }
    

    return 0;
}

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len, char toSend[]){
    int rc = 0;
    char buffer[18];
    char bufferIn[12];



    printf("Enter a message\n");
    int length = strlen(toSend)-1;

    int Index1 = 0;
    int Index2 = 1;

    int seqNumberACK = 0;
    int sequenceNumberLatest = 0;


    time_t sentTime;
    time_t currentTime;
    while(Index1 < strlen(toSend)-1){
        memset (buffer, 0, 17);
        if (Index2 >= strlen(toSend)-1){
            sprintf(buffer, "%11d%4d%c", sequenceNumberLatest,1,toSend[Index1]);
        }else{
            sprintf(buffer, "%11d%4d%c%c", sequenceNumberLatest,2,toSend[Index1], toSend[Index2]);
        }
        
        printf("Index1:  %d,", Index1);
        printf("buffer %s\n", buffer);

        
        int networkLength = (strlen(buffer));
        //int flags = 0;

        memset (bufferIn, 0, 11);

        //printf("Getting ready to send :D...\n");

        sentTime = time(NULL);
        rc = sendto(sd, &buffer, networkLength, MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));
        //printf("SENT!!!\n");

        if (rc < 0){
            perror("Error w/ sendto");
            return 0;
        }

        rc = recvfrom(sd, &bufferIn, 12, MSG_DONTWAIT, (struct sockaddr *)&from_address,&from_address_len);

        if (rc > 0){
            //printf("RC: %d\n", rc);
            //printf("BufferIN: %s", bufferIn);
            sscanf(bufferIn, "%11d", &seqNumberACK);
            //printf("ACK# Received: %d\n", seqNumberACK);
            //printf("Current ACK#: %d\n", sequenceNumberLatest);
            if (seqNumberACK == sequenceNumberLatest){
                Index1+=2;
                Index2+=2;
                sequenceNumberLatest+=2;
            }
        }
        currentTime = time(NULL);
        printf("Time delay: %ld \n", currentTime-sentTime);
        if (currentTime - sentTime > 2){
            continue;
        }        

    }

    return 1;

};



