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

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len);

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

    if (connect(sd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) {
        close(sd);
        perror("error connecting stream socket");
        exit(1);
    }

    for(;;){
        sendData(sd, server_address, from_address, from_address_len);
    }


    return 0;
}

int sendData(int sd, struct sockaddr_in server_address, struct sockaddr_in from_address, socklen_t from_address_len){
    int rc = 0;
    char buffer[18];
    char bufferIn[BUFFSIZE];
    char message[100];
    int maxLeng = 40;
    char *strPtr1;
    char *strPtr2;


    int sequenceNumber = 0;

    printf("Enter a message\n");
    char *ptr = fgets(message, maxLeng, stdin);
    int length = strlen(message);

    int finalSequenceNumber = sequenceNumber + length;

    sprintf(buffer, "%11d%4d%2s", finalSequenceNumber,length,"US");

    printf("String Length: %d\n", length);

    if(ptr == NULL){
        perror("Error w/ fgets");
        return 0;
    }

    int networkLength = htons(strlen(message));


    rc = sendto(sd, buffer, networkLength, MSG_DONTWAIT, (struct sockaddr *) &server_address,  sizeof(server_address));

    if (rc < 0){
        perror("Error w/ sendto");
        return 0;
    }

    return 0;

};



