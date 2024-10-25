#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>     // Import for `printf` & `perror`
#include <unistd.h> 
#include <time.h>   // Import for `read`, `write & `lseek`
#include <string.h>    // Import for string functions
#include <stdbool.h>    // Import for `bool` data type
#include <sys/types.h> // Import for `open`, `lseek`
#include <sys/stat.h>  // Import for `open`
#include <fcntl.h>     // Import for `open`
#include <stdlib.h>    // Import for `atoi`
#include <errno.h>     // Import for `errno`
#include "../fun/constants.h"
#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"

// Function Prototypes =================================
bool authenticate_customer(int id, const char* password, struct Customer* loggedInCustomer);
bool login_handler(int connFD, struct Customer *ptrToCustomer);
bool get_account_details(int connFD, struct Account *customerAccount);
bool get_customer_details(int connFD, int customerID);
//bool get_transaction_details(int connFD, struct Transaction *trans);
bool get_transaction_detail(int connFD, int accountNumber);


// Function to authenticate a customer by checking their ID and password in the file
bool authenticate_customer(int id, const char *password, struct Customer *ptrToCustomer) {
    FILE *file = fopen("custom.txt", "r");
    if (!file) {
        perror("Could not open customer file");
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        struct Customer tempCustomer;
        // Parse customer data from each line in the file
        if (sscanf(line, "%d,%24[^,],%c,%d,%29[^,],%d,%d",
                   &tempCustomer.id,
                   tempCustomer.name,
                   &tempCustomer.gender,
                   &tempCustomer.age,
                   tempCustomer.password,
                   &tempCustomer.account,
                   &tempCustomer.active) == 7) {
            // Check if ID and password match
            if (tempCustomer.id == id && strcmp(tempCustomer.password, password) == 0 && tempCustomer.active == 1) {
                *ptrToCustomer = tempCustomer; // Copy matched customer data
                fclose(file);
                return true; // Authentication successful
            }
        }
    }
    fclose(file);
    return false; // Authentication failed
}

// Function to handle login
bool login_handler(int connFD, struct Customer *ptrToCustomer) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    // Prompt for login ID
    bzero(writeBuffer, sizeof(writeBuffer));
    //strcpy(writeBuffer, CUSTOMER_LOGIN_PROMPT_ID);
    writeBytes = write(connFD,"&Enter the login id \n", strlen("&Enter the login id \n"));
    printf("sent \n");
    if (writeBytes == -1) {
        perror("Error while writing login ID prompt to client!");
        return false;
    }

    // Read login ID
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error while reading login ID from client!");
        return false;
    }
    int id = atoi(readBuffer); // Convert login ID to integer
    printf("%s \n",readBuffer);

    // Prompt for password
    bzero(writeBuffer, sizeof(writeBuffer));
    //strcpy(writeBuffer, CUSTOMER_LOGIN_PROMPT_PASSWORD);
    writeBytes = write(connFD,"&Enter the password\n", strlen("&Enter the password\n"));
    if (writeBytes == -1) {
        perror("Error while writing password prompt to client!");
        return false;
    }

    // Read password
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error while reading password from client!");
        return false;
    }
    printf("%s \n",readBuffer);
    char password[1000];
    strcpy(password, readBuffer);
    password[strcspn(password, "\n")] = '\0';  // Remove newline character

    // Authenticate customer by checking the ID and password from the file
    if (authenticate_customer(id, password, ptrToCustomer)) {
        bzero(writeBuffer, sizeof(writeBuffer));
        //strcpy(writeBuffer, CUSTOMER_LOGIN_SUCCESS);
        //writeBytes = write(connFD,"*Login success\n", strlen("*Login success\n"));
        if (writeBytes == -1) {
            perror("Error while writing login success message to client!");
            return false;
        }
        return true; // Successful login
    } else {
        bzero(writeBuffer, sizeof(writeBuffer));
        //strcpy(writeBuffer, CUSTOMER_LOGIN_FAILED);
        writeBytes = write(connFD,"*Login failed\n", strlen("*Login failed\n"));
        if (writeBytes == -1) {
            perror("Error while writing login failure message to client!");
        }
        return false; // Login failed
    }
}


// Assume a pre-defined Account structure



bool get_account_details(int connFD, struct Account *customerAccount) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    int accountNumber;
    struct Account account;
    FILE *file;

    // If customerAccount is NULL, ask client for account number
    if (customerAccount == NULL) {
        writeBytes = write(connFD, GET_ACCOUNT_NUMBER, strlen(GET_ACCOUNT_NUMBER));
        if (writeBytes == -1) {
            perror("Error writing GET_ACCOUNT_NUMBER message to client!");
            return false;
        }

        // Read the account number from the client
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading account number from client!");
            return false;
        }

        accountNumber = atoi(readBuffer);  // Convert account number to integer
    } else {
        accountNumber = customerAccount->accountNumber;  // Use provided account number
    }

    // Open the account file in read mode
    file = fopen("acc.txt", "r");
    if (file == NULL) {
        perror("Error opening account file!");
        //writeBytes = write(connFD, ACCOUNT_ID_DOESNT_EXIST, strlen(ACCOUNT_ID_DOESNT_EXIST));
        return false;
    }

    // Initialize a buffer to store each line
    char line[256];
    bool accountFound = false;

    // Read through each line in the file
    while (fgets(line, sizeof(line), file) != NULL) {
        struct Account tempAccount;
        int activeInt;

        // Parse the line using sscanf
        int parsedFields = sscanf(line, "%d,%d,%d,%ld,%d",
                                  &tempAccount.accountNumber,
                                  &tempAccount.customerid,
                                  &activeInt,
                                  &tempAccount.balance,
                                  &tempAccount.transactionCount);

        tempAccount.active = (bool)activeInt;

        // Check if all fields were successfully parsed and if the account number matches
        if (parsedFields == 5 && tempAccount.accountNumber == accountNumber) {
            account = tempAccount;
            accountFound = true;
            break;
        }
    }

    // Close the file after reading
    fclose(file);

    // If account not found, send error message to client
    if (!accountFound) {
        //writeBytes = write(connFD, ACCOUNT_ID_DOESNT_EXIST, strlen(ACCOUNT_ID_DOESNT_EXIST));
        return false;
    }

    // Copy account details to the provided customerAccount struct
    if (customerAccount != NULL) {
        *customerAccount = account;
    }

    return true;
}
bool get_customer_details(int connFD, int customerID) {
    ssize_t readBytes, writeBytes;           // Bytes read from/written to socket
    char writeBuffer[1000];                  // Buffer for socket communication
    struct Customer customer;                // Variable to hold customer information
    int customerFileDescriptor;

    // Open the customer file to read the customer details
    customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileDescriptor == -1) {
        perror("Error opening customer file!");
        writeBytes = write(connFD, CUSTOMER_ID_DOES_NOT_EXIST, strlen(CUSTOMER_ID_DOES_NOT_EXIST));
        return false;
    }

    // Seek to the location of the customer record in the file
    int offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1) {
        if (errno == EINVAL) {
            perror("Invalid customer ID provided!");
            writeBytes = write(connFD, CUSTOMER_ID_DOES_NOT_EXIST, strlen(CUSTOMER_ID_DOES_NOT_EXIST));
        } else {
            perror("Error seeking to the customer record!");
        }
        close(customerFileDescriptor);
        return false;
    }

    // Lock the record for reading
    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
    if (fcntl(customerFileDescriptor, F_SETLKW, &lock) == -1) {
        perror("Error obtaining read lock on the customer record!");
        close(customerFileDescriptor);
        return false;
    }

    // Read the customer details from the file
    readBytes = read(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1) {
        perror("Error reading customer record from file!");
        lock.l_type = F_UNLCK;  // Unlock before returning
        fcntl(customerFileDescriptor, F_SETLK, &lock);
        close(customerFileDescriptor);
        return false;
    }

    // Unlock the record after reading
    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);
    close(customerFileDescriptor);

    // If the customer is inactive, send a message to the client
    if (!customer.active) {
        writeBytes = write(connFD, ACCOUNT_INACTIVE, strlen(ACCOUNT_INACTIVE));
        return true;  // Customer exists but is inactive
    }

    // Prepare the response with customer details (excluding password for security)
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Customer Details - \n\tCustomer ID : %d\n\tAccount Number : %d\n\tCustomer Status : %s\n", 
            customer.id, 
            customer.account, 
            (customer.active ? "Active" : "Inactive"));

    
    // Send response to client
    strcat(writeBuffer, "^");  // Append terminator for client processing
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing customer details to client!");
        return false;
    }

    return true;
}

bool get_transaction_detail(int connFD, int accountNumber)
{
    char buffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = accountNumber;

    // Get the account details first
    if (get_account_details(connFD, &account))
    {
        if (!account.active)
        {
            // If the account is deactivated, notify the client
            writeBytes = write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
            memset(buffer,'\0',sizeof(buffer));
            readBytes = read(connFD, buffer, sizeof(buffer)); // Dummy read
            return false;
        }

        // Account is active, proceed with fetching transaction details
         FILE *file = fopen("trans.txt", "r");
    if (!file) {
        perror("Could not open transaction file");
        return false;
    }

    char line[256];
    bool transactionFound = false;

    // Read each line and parse the transaction details
    while (fgets(line, sizeof(line), file)) {
        struct Transaction transaction;
        struct tm tm;
        char dateBuffer[30];

        // Parse transaction data from line
        if (sscanf(line, "%d,%d,%d,%ld,%ld,%29[^\n]",
                   &transaction.transactionID,
                   &transaction.accountNumber,
                   (int *)&transaction.operation,
                   &transaction.oldBalance,
                   &transaction.newBalance,
                   dateBuffer) == 6) {

            // // Convert date from string to time_t
            // if (strptime(dateBuffer, "%Y-%m-%d %H:%M:%S", &tm) != NULL) {
            //     transaction.transactionTime = mktime(&tm);
            // } else {
            //     fprintf(stderr, "Failed to parse date for transaction ID %d\n", transaction.transactionID);
            //     continue;
            // }

            // Check if the transaction is for the specified account number
            if (transaction.accountNumber == accountNumber) {
                // Transaction found for this account, format the transaction details
                char transactionDetails[500];
                sprintf(transactionDetails, "Transaction ID: %d\nOld Balance: ₹ %ld\nNew Balance: ₹ %ld\nOperation: %s\nDate: %s\n\n",
                        transaction.transactionID,
                        transaction.oldBalance,
                        transaction.newBalance,
                        (transaction.operation ? "Deposit" : "Withdraw"),
                        ctime(&transaction.transactionTime));

                strcat(buffer, transactionDetails);
                transactionFound = true;
            }
        }
    }

    fclose(file);
    //return transactionFound;
        // Check if any transactions were found
        if (transactionFound)
        {
            
            writeBytes = write(connFD, buffer, strlen(buffer));
            if (writeBytes == -1)
            {
                perror("Error writing transaction details to client!");
                return false;
            }
        }
        else
        {
            // If no transactions were found
            writeBytes = write(connFD, NO_TRANSACTIONS, strlen(NO_TRANSACTIONS));
            if (writeBytes == -1)
            {
                perror("Error writing NO_TRANSACTIONS message to client!");
                return false;
            }
        }
        memset(buffer,'\0',sizeof(buffer));
        readBytes = read(connFD, buffer, sizeof(buffer)); // Dummy read
        return true;
    }
    else
    {
        // Error getting account details
        writeBytes = write(connFD, ACCOUNT_DETAILS_ERROR, strlen(ACCOUNT_DETAILS_ERROR));
        readBytes = read(connFD, buffer, sizeof(buffer)); // Dummy read
        return false;
    }
} 
#endif