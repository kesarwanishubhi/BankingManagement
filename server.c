#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h> 
#include <arpa/inet.h>
#include "./fun/customer.h"
#include "./fun/employee.h"
#include "./fun/manager.h"
#include "./fun/administrator.h"

#define PORT 9097
#define BUFFER_SIZE 1024

// Function prototypes
void *handleClient(void *socket_desc);


int main() {
    int server_fd, client_sock, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections...\n");


    // Accept incoming connections
    while ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen))) {
        printf("Connection accepted.\n");

        // Allocate memory for the new client socket
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        // Create a new thread for each client connection
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handleClient, (void *)new_sock) < 0) {
            perror("Could not create thread");
            close(client_sock);
            free(new_sock);
            continue;
        }

        printf("Handler assigned.\n");
    }

    if (client_sock < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    return 0;
}

// Thread function to handle each client
void *handleClient(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int choice;

    // Read the user's choice from the client
    bzero(buffer,sizeof(buffer));
    read(client_sock, buffer, BUFFER_SIZE);
    printf("Received role choice: %s\n", buffer); // Debugging line
    choice = atoi(buffer);  // Convert the string choice to an integer

    // Handle the user's role based on the choice using switch-case
    switch (choice) {
        case 1:
            //char message[] = "Customer Menu: \n1. View Account Balance\n2. Deposit Money\n3. Withdraw Money\n4. Transfer Funds\n5. Apply for a Loan\n Enter the choice\n";
            //write(client_sock, message, strlen(message));
            customer_operation_handler(client_sock);
            break;
        case 2:
            //char message[] = "Bank Employee Menu: \n1. Add New Customer\n2. Modify Customer Details\n3. Process Loan Applications\n4. View Customer Transactions\n Enter the choice\n";
            //write(client_sock, message, strlen(message));
            //printf("Hello \n");
            empl_handler(client_sock);
            
            break;
        case 3:
            //char message[] = "Manager Menu: \n1. Activate/Deactivate Customer Accounts\n2. Assign Loan Applications\n3. Review Customer Feedback\n Enter the choice\n";
            //write(client_sock, message, strlen(message));
            managerLogin(client_sock);
            //printf("Hello");
            break;
        case 4:
            //char message[] = "Administrator Menu: \n1. Add New Bank Employee\n2. Modify Customer/Employee Details\n3. Manage User Roles\n Enter the choice\n";
            //write(client_sock, message, strlen(message));
            //handleAdministrator(client_sock);
            adminLogin(client_sock);
            //printf("Hello");
            break;
        default:
            write(client_sock, "*Invalid choice. Please try again.\n", strlen("*Invalid choice. Please try again.\n"));
            break;
    }

    // Close the socket and free the memory
    close(client_sock);
    free(socket_desc);
    pthread_exit(NULL);
    return NULL;
}

