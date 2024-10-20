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



#define PORT 9097
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

    // Select User Role
    printf("Select User Role:\n");
    printf("1. Customer\n");
    printf("2. Bank Employee\n");
    printf("3. Manager\n");
    printf("4. Administrator\n");
    printf("Enter the choice: ");
    fgets(choice, sizeof(choice), stdin);
    choice[strcspn(choice, "\n")] = '\0'; // Remove newline character

    // Send user role choice to server
    write(sock, choice, strlen(choice));
    do{
        // printf("menu");
        fflush(stdout);
        fflush(stdin);
        bzero(readbuf, sizeof(readbuf));
        bzero(writebuf, sizeof(writebuf));
        readBytes=read(sock,readbuf,sizeof(readbuf));
        if(readBytes==-1){
            printf("Error \n");
        }
        else if(readBytes==0){
            printf("Closing connection \n");
        }
        else{
            if(strchr(readbuf,'*')!=NULL){
                printf("%s \n",readbuf);
                continue;
            }
            else {
                printf("%s \n", readbuf);
                fflush(stdout);
                fflush(stdin);
                fgets(writebuf,sizeof(writebuf),stdin);
                fflush(stdout);
                fflush(stdin);
            }
            writeBytes=write(sock,writebuf,sizeof(writebuf));
        }
        // printf("came here");

    }while(readBytes>0);



    // // Wait for server prompt for login ID
    // // memset(buffer, 0, sizeof(buffer));
    // bzero(buffer, sizeof(buffer));
    // int valread = read(sock, buffer, sizeof(buffer) - 1, 0);  // Receive server's prompt for login ID
    // if (valread > 0) {
    //     buffer[valread] = '\0';  // Null-terminate the received data
    //     printf("%s", buffer);  // Display server's prompt for login ID
    // }

    // // Send login ID to server
    // bzero(message, sizeof(message));
    // fgets(message, sizeof(message), stdin);
    // message[strcspn(message, "\n")] = 0;  // Remove newline character
    // send(sock, message, strlen(message), 0);

    // // Wait for server prompt for password
    // // memset(buffer, 0, sizeof(buffer));
    // bzero(buffer, sizeof(buffer));
    // valread = read(sock, buffer, sizeof(buffer) - 1, 0);  // Receive server's prompt for password
    // if (valread > 0) {
    //     buffer[valread] = '\0';  // Null-terminate the received data
    //     printf("%s", buffer);  // Display server's prompt for password
    // }

    // // Send password to server
    // bzero(message, sizeof(message));
    // fgets(message, sizeof(message), stdin);
    // message[strcspn(message, "\n")] = 0;  // Remove newline character
    // send(sock, message, strlen(message), 0);

    // // After authentication, you can enter the menu communication loop
    // while (1) {
    //     // memset(buffer, 0, sizeof(buffer));  // Clear buffer
    //     bzero(buffer, sizeof(buffer));
    //     valread = read(sock, buffer, sizeof(buffer) - 1, 0);
    //     printf("received %s",buffer );  // Receive server's menu
    //     if (valread > 0) {
    //         buffer[valread] = '\0';  // Null-terminate the received data
    //           // Display server's menu
    //     }
    //    // char *message = strtok(buffer, "##");

    //    bzero(message, sizeof(message));
    //   while (message != NULL) {
    //         // Check if it's an informational message
    //         bzero(message, sizeof(message));
    //         printf("Inside message while");
            
    //         if (strncmp(message, "INFO:", 5) == 0) {
    //             printf("We are here");
    //             // Strip off the "INFO:" prefix and display the actual message
    //             printf("%s\n", message + 5);
    //             printf("Hey\n"); 
    //             // message = strtok(NULL, "##");
    //             // printf("%s",message);
                
    //             }
    //         else {
    //             printf("We should be here");
    //             // Print the message as it is (like the menu)
    //             printf("%s\n", message);
    //             bzero(input, sizeof(input));
    //             // memset(input, 0, sizeof(input));  // Clear message buffer
    //           fgets(input, sizeof(input), stdin);  // Get user input
    //           input[strcspn(input, "\n")] = 0;  // Remove newline character
    //         send(sock, input, strlen(input), 0);  // Send input to server
    //          printf("SENT \n");
    //          //message = strtok(NULL, "##");
    //         }

    //         // Move to the next message part if any
            
    //     }
    //     // Now, take input from the user
        
    //     // if (strcmp(message, "exit") == 0) {  // Exit condition
    //     //     break;
    //     // }
    // }

    // Close the socket after loop ends
    close(sock);
    return 0;
}