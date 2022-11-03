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
    
    sendData(sd, server_address, from_address, from_address_len, message);

    

    return 0;
}

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len, char toSend[]){
    int rc = 0;
    char buffer[18];
    char bufferIn[12];


    int length = strlen(toSend)-1;

    int Index1 = 0;
    int Index2 = 1;

    int Index1UnACKED = 0;
    int Index2UnACKED = 1;

    int lastUNACKEDSeqNumber = 0;
    int seqNumberACK = 0;
    int sequenceNumberLatest = 0;

    time_t sentTime;
    time_t currentTime;

    while(Index1 < strlen(toSend)-1){
        memset (buffer, 0, 17);
        int i = 0;
        //printf("Latest SEQUENCE NUMBER: %d LAST unacked number: %d \n",sequenceNumberLatest, lastUNACKEDSeqNumber);
        while(sequenceNumberLatest - lastUNACKEDSeqNumber<=8){
            if (Index2 >= strlen(toSend)-1){
                sprintf(buffer, "%11d%4d%c", sequenceNumberLatest,1,toSend[Index1]);
            }else{
                sprintf(buffer, "%11d%4d%c%c", sequenceNumberLatest,2,toSend[Index1], toSend[Index2]);
            }
            int networkLength = (strlen(buffer));
            sentTime = time(NULL);
            rc = sendto(sd, &buffer, networkLength, MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));
            Index1+=2;
            Index2+=2;
            sequenceNumberLatest+=2;
            //printf("%d\n", sequenceNumberLatest);
        }
        
        //printf("Index1:  %d,", Index1);
        //printf("buffer %s\n", buffer);


        memset (bufferIn, 0, 11);

        //sentTime = time(NULL);
        //rc = sendto(sd, &buffer, networkLength, MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));
        /*
        if (rc < 0){
            perror("Error w/ sendto");
            return 0;
        }
        */
        

        rc = recvfrom(sd, &bufferIn, 12, MSG_DONTWAIT, (struct sockaddr *)&from_address,&from_address_len);

        if (rc > 0){
            sscanf(bufferIn, "%11d", &seqNumberACK);
            //printf("SEQUENCE NUMBER ACK: %d LAST unacked number: %d \n",seqNumberACK, lastUNACKEDSeqNumber);
            //printf("bufferIN: %s\n", bufferIn);
            if (seqNumberACK == lastUNACKEDSeqNumber){
                lastUNACKEDSeqNumber+=2;
            }
        }else{
            currentTime = time(NULL);
            //printf("Time delay: %ld \n", currentTime-sentTime);
            //printf("BRUH MOMENTO IT DROPPED A PACKET :SCREAM: \n");
            if (currentTime - sentTime > 2){
                //printf("BRUH MOMENTO IT DROPPED A PACKET :SCREAM: \n");
                Index1 = lastUNACKEDSeqNumber;
                sequenceNumberLatest = lastUNACKEDSeqNumber;
                Index2 = Index1+1;  
            }
        }
        //currentTime = time(NULL);
    }
    return 1;

};



