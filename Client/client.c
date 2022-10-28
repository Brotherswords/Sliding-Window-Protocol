#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>



int getFileLength(FILE* fp);

int main(int argc, char *argv[]){
    int sd;
    struct sockaddr_in server_address;
    char filename[100];
    int portNumber;
    char serverIP[29];
    FILE *inputFile;
    int rc = 0;

    if (argc < 3) {
        printf("Usage: %s <server ip> <port>\n", argv[0]);
        exit(1);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);

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


    while(1){
        //Making sure that filename doesnt have extra characters
        memset(filename,0,100);
        printf("What is the name of the file you'd like to send?\n");
        //Take user input on the file name and store it!
        scanf("%s", filename);
        //Create an input file string of only that length and copy it over to prevent extra characters from being sent over the network.
        char inputFileName[strlen(filename)];
        memcpy(inputFileName, filename, strlen(filename));
        inputFileName[strlen(filename)] = '\0';
        printf("filename to send it '%s'\n", inputFileName);
        printf("filename size to send it '%lu'\n", strlen(inputFileName));
        //Get and convert the size of the file to network order.
        int sizeOfFileName = strlen(inputFileName);
        int converted_sizeOfFileName = ntohs(sizeOfFileName);

        //WRITE STATEMENT
        //Getting ready to send the size of the file name in bytes
        rc = write(sd, &converted_sizeOfFileName, sizeof(converted_sizeOfFileName));
        printf("wrote %d bytes to send the filename size\n", rc);
        if (rc < 0) {
            perror("error writing");
        }

        //Check if DONE is entered, and if so write the corresponding file size over to make sure the 
        //reads on the server size understand that this is not a file to be transferred, and then terminate
        //the connection. 
        int isDone = -1;
        isDone = strcmp(inputFileName, "DONE");
        if (isDone == 0) {
            int bytesToWrite = 0;
            int convertedBytesToWrite = htonl(bytesToWrite);
            rc = write(sd, &convertedBytesToWrite, sizeof(convertedBytesToWrite));
            rc = write(sd, &inputFileName, sizeof(inputFileName));
            printf("Connection Closed :)");
            return 0;
        };    
        //Getting ready to read from the file. 
        if((inputFile = fopen(filename, "rb")) == NULL) {
            printf("cannot open input file %s\n", filename);
        }

        //Get and convert the size of the file to network length
        int bytesToWrite = getFileLength(inputFile);
        int convertedBytesToWrite = htonl(bytesToWrite);
        rc = write(sd, &convertedBytesToWrite, sizeof(convertedBytesToWrite));
        printf("wrote %d bytes to send the file length\n", bytesToWrite);
        if(rc < 0){ 
            perror("error writing");
        }

        //WRITE STATEMENT
        //Getting ready to send the file name in bytes
        rc = write(sd, &inputFileName, sizeof(inputFileName));
        if(rc < 0){ 
            perror("error writing");
        }


        //WRITE STATEMENT
        //Send contents of the file to the server one byte a a time!
        //Total numberOfBytes written
        int totalBytesRead = 0;
        char byte;
        while (1)
        {
            //get next byte from the InputFile
            byte= fgetc (inputFile); // reading the file one char at a time, if it fails to get anything it'll return an EOF, which indicates end of file!
            //Check if we've reached the end of the file, if yes that means we're done and time to break out of the loop.
            if (byte == EOF){
                break;
            }
            //Write the byte to the server
            rc = write(sd, &byte, 1);
            //increment number of bytes
            totalBytesRead += 1;
        }
        //close the file that we've been reading from.
        printf("Closing the file test.c\n") ;
        fclose (inputFile ) ;
        //READ STATEMENT ACK from Server making sure the server has recieved the proper number of bytes :)
        int totalBytesReceived = 0;
        rc = read(sd, &totalBytesReceived, sizeof(int));
        printf("ACK from server: Server Received %d bytes\n", totalBytesReceived);
    }

    return 0;
}

int getFileLength(FILE* fp){
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}



