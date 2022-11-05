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

    //setting up the UDP socket
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    portNumber = strtol(argv[2], NULL,10);
    strcpy(serverIP, argv[1]);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = inet_addr(serverIP);

    //Setting up the message variable and the maxLeng possible
    char message[100];
    int maxLeng = 100;

    printf("TEST MESSAGE #1: How are you today? I'm fine thanks for asking me\n");
    printf("TEST MESSAGE #2: How are you today? I'm fine thanks for asking me!\n");

    printf("Enter a message\n");
    char *ptr = fgets(message, maxLeng, stdin);
    int length = strlen(message)-1;

    int networkLength = ntohl(length);

    rc = sendto(sd, &networkLength, sizeof(int), MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));
    
    sendData(sd, server_address, from_address, from_address_len, message);

    return 0;
}

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len, char toSend[]){
    //Setting up variables for return code, output buffer and the input buffer for the ACKs
    int rc = 0;
    char buffer[18];
    char bufferIn[12];

    //Length of the varibable
    int length = strlen(toSend)-1;

    //The Indexes of what to send next
    int Index1 = 0;
    int Index2 = 1;

    //The last sequence number that hasn't been acked, the sequence number recieved from the server, latest sequence number sent.
    int lastUNACKEDSeqNumber = 0;
    int seqNumberACK = 0;
    int sequenceNumberLatest = 0;

    //declaring current and sent times for timeout
    time_t sentTime;
    time_t currentTime;

    //This will allow packets to be sent until the last packet has been sent; marked by index1.
    while(Index1 <= strlen(toSend)-1 || lastUNACKEDSeqNumber < length){
        //clearing the buffer each time
        memset (buffer, 0, 17);
        int i = 0;
        //printf("Latest SEQUENCE NUMBER: %d LAST unacked number: %d \n",sequenceNumberLatest, lastUNACKEDSeqNumber);
        //Maintains a window size of 10 since each sequence number precedes the next two packets.
        while(sequenceNumberLatest - lastUNACKEDSeqNumber<=8){
            //decides whether to send one packet or two packets.
            //printf("Latest SEQUENCE NUMBER: %d LAST unacked number: %d Index 1: %d Letters being sent:%c%c \n",sequenceNumberLatest, lastUNACKEDSeqNumber,Index1,toSend[Index1],toSend[Index2]);
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
        }

        //Clear the ack buffer
        memset (bufferIn, 0, 11);
        
        //receives the ack 
        rc = recvfrom(sd, &bufferIn, 12, MSG_DONTWAIT, (struct sockaddr *)&from_address,&from_address_len);

        //Check if the ACK is valid and act accordingly
        if (rc > 0){
            sscanf(bufferIn, "%11d", &seqNumberACK);
            if (seqNumberACK == lastUNACKEDSeqNumber){
                lastUNACKEDSeqNumber+=2;
            }
        }else{
            currentTime = time(NULL);
            //printf("Time delay: %ld \n", currentTime-sentTime);
            if (currentTime - sentTime > 2){
                //printf("BRUH MOMENTO IT DROPPED A PACKET :SCREAM: \n");
                Index1 = lastUNACKEDSeqNumber;
                sequenceNumberLatest = lastUNACKEDSeqNumber;
                Index2 = Index1+1;  
            }
        }

    }
    printf("Sent Everything!\n");
    return 0;

};



