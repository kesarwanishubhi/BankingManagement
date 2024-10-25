#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "./fun/customer.h"
#include "./fun/employee.h"
#include "./fun/constants.h"



#define PORT 9098
#define BUFFER_SIZE 1024
//void connection_handler(int sockFD);



int main() {
    int sock;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    char readbuf[BUFFER_SIZE];
    char writebuf[BUFFER_SIZE];
    ssize_t readBytes,writeBytes;
    char choice[50];
    char server_message[2000], client_message[2000];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    address.sin_family = AF_INET;  // Server address
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return -1;
    }

    int status = connect(sock, (struct sockaddr*)&address, sizeof(address));
    if (status < 0) {
        perror("Connection establishment failed");
        return -1;
    }
    send(sock,"hey",strlen("hey"),0);
    
    recv(sock,input,BUFFER_SIZE - 1,0);
    printf("%s",input);
    // Select User Role
    printf("Select User Role:\n");
    printf("1. Customer\n");
    printf("2. Bank Employee\n");
    printf("3. Manager\n");
    printf("4. Administrator\n");
    printf("Enter the choice: ");
    fgets(choice, 50, stdin);
    //choice[strcspn(choice, "\n")] = '\0'; // Remove newline character
    send(sock, choice, strlen(choice),0);
    printf("I send");

//     //username
//     fflush(stdout);
//     fflush(stdin);
//     memset(server_message,'\0',sizeof(server_message));
//     memset(client_message,'\0',sizeof(client_message));
//     recv(sock,server_message,sizeof(server_message),0);
//     printf("\n %s", server_message);
//     scanf("%s",client_message);
//     send(sock, client_message,strlen(client_message),0);

   

//     //password
//     fflush(stdout);
//     fflush(stdin);
//     memset(server_message,'\0',sizeof(server_message));
//     memset(client_message,'\0',sizeof(client_message));
//     recv(sock,server_message,sizeof(server_message),0);
//     printf("\n%s", server_message);
//     scanf("%s",client_message);
//     send(sock, client_message,strlen(client_message),0);

//     //prompt
//     // fflush(stdout);
//     // fflush(stdin);
//     // memset(server_message,'\0',sizeof(server_message));
//     // memset(client_message,'\0',sizeof(client_message));
//     // recv(sock,server_message,sizeof(server_message),0);
//     // printf("\n %s \n", server_message);
//     //scanf("%s",client_message);
//     //send(sock, client_message,strlen(client_message),0);

//     // Send user role choice to server
    
    do {
    fflush(stdout);
    fflush(stdin);
    memset(readbuf, '\0', sizeof(readbuf));
    
    readBytes = recv(sock, readbuf, sizeof(readbuf) - 1, 0);
    fflush(stdout);
    fflush(stdin);
    if (readBytes <= 0) {
        if (readBytes == 0) {
            printf("Connection closed\n");
        } else {
            perror("recv error");
        }
        break;
    }

    readbuf[readBytes] = '\0'; // Null terminate the received string

    if (strchr(readbuf, '*') != NULL) {
        fflush(stdout);
        fflush(stdin);
        printf("Received special message: %s\n", readbuf);
        continue;
    } else {
        printf("I came &");
        printf("%s \n", readbuf);
        fflush(stdout);
        fflush(stdin);
        scanf("%s", writebuf);// Ensure input is sanitized and not larger than expected
        printf("%s",writebuf);
        writeBytes = send(sock, writebuf, strlen(writebuf), 0); // Send the length of the string
        printf("I sent\n");
        if (writeBytes < 0) {
            perror("send error");
        }
    }

} while (true);



    close(sock);
    return 0;
}
