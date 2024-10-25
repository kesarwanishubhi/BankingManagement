#ifndef CUSTOMER_H
#define CUSTOMER_H


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdbool.h> 
#include "../fun/common.h"

#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"


#include "../fun/constants.h"


struct Customer loggedInCustomer;
int semIdentifier;


bool customer_operation_handler(int connFD);
bool loan(int connFD);
bool transfer_funds(int connFD);
bool view_transaction_history(int connFD);
bool deposit(int connFD);
bool withdraw(int connFD);
bool get_balance(int connFD);
bool change_password(int connFD);
bool lock_critical_section(struct sembuf *semOp);
bool unlock_critical_section(struct sembuf *sem_op);
void write_transaction_to_array(int *transactionArray, int ID);
int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation);
int generate_unique_feedback_id();
int log_feedback_to_file(int accountNumber, const char *feedbackText);
//bool history(int connFD);
bool feedback(int connFD);
int generate_feedback_id();
int generate_loan_id();
int log_loan_application(int accountNumber, long int loanAmount, const char *loanPurpose);

// =====================================================

// Function Definition =================================


bool customer_operation_handler(int connFD)
{
    if (login_handler(connFD, &loggedInCustomer))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client

        // // Get a semaphore for the user
        // key_t semKey = ftok(CUSTOMER_FILE, loggedInCustomer.account); // Generate a key based on the account number hence, different customers will have different semaphores

        // union semun
        // {
        //     int val; // Value of the semaphore
        // } semSet;

        // int semctlStatus;
        // semIdentifier = semget(semKey, 1, 0); // Get the semaphore if it exists
        // if (semIdentifier == -1)
        // {
        //     semIdentifier = semget(semKey, 1, IPC_CREAT | 0700); // Create a new semaphore
        //     if (semIdentifier == -1)
        //     {
        //         perror("Error while creating semaphore!");
        //         _exit(1);
        //     }

        //     semSet.val = 1; // Set a binary semaphore
        //     semctlStatus = semctl(semIdentifier, 0, SETVAL, semSet);
        //     if (semctlStatus == -1)
        //     {
        //         perror("Error while initializing a binary sempahore!");
        //         _exit(1);
        //     }
        // }

        // bzero(writeBuffer, sizeof(writeBuffer));
        // strcpy(writeBuffer, CUSTOMER_LOGIN_SUCCESS);
        bool logged_in = true;
        while (logged_in)
        {
            
            writeBytes = write(connFD,CUSTOMER_MENU, strlen(CUSTOMER_MENU));
            if (writeBytes == -1)
            {
                perror("Error while writing CUSTOMER_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for CUSTOMER_MENU");
                return false;
            }
            
            printf("READ BUFFER : %s\n", readBuffer);
            int choice = atoi(readBuffer);
            // printf("CHOICE : %d\n", choice);
            switch (choice)
            {
            case 1:
                view_transaction_history(connFD);
                break;
            case 2:
                deposit(connFD);
                break;
            case 3:
                withdraw(connFD);
                break;
            case 4:
                get_balance(connFD);
                break;
            case 5:
                //get_transaction_detail(connFD, loggedInCustomer.account);
                break;
            case 6:
                change_password(connFD);
                break;
            case 7:
                feedback(connFD);
                break;
            
            case 8:
                loan(connFD);
                break;
            case 9:
                logged_in=false;
                break;
            

            default:
                writeBytes = write(connFD, WRONG_CHOICE, strlen(WRONG_CHOICE));//invalid choice message
                return false;
            }
        }
    }
    else
    {
        // CUSTOMER LOGIN FAILED
        return false;
    }
    return true;
}
bool deposit(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;
    printf("ACcount number %d\n",account.accountNumber);

    long int depositAmount = 0;

    
    if (get_account_details(connFD, &account))
    {
        if (account.active)
        {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, DEPOSIT_AMOUNT, sizeof(writeBuffer));
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing DEPOSIT_AMOUNT to client!");
                //unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading deposit money from client!");
                //unlock_critical_section(&semOp);
                return false;
            }

            depositAmount = atol(readBuffer);
            if (depositAmount != 0)
            {
                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance + depositAmount, 1);
                // write_transaction_to_array(account.transactions, newTransactionID);
                // account.transactionCount++;

                account.balance += depositAmount;
                  FILE *file = fopen("acc.txt", "r+");
    if (!file) {
        perror("Could not open account file");
        return false;
    }


               char line[256];
    char updatedContent[4096] = "";
    bool accountUpdated = false;

    while (fgets(line, sizeof(line), file)) {
        struct Account tempAccount;
        int parsedFields = sscanf(line, "%d,%d,%d,%ld,%d",
                                  &tempAccount.accountNumber,
                                  &tempAccount.customerid,
                                  (int*)&tempAccount.active,
                                  &tempAccount.balance,
                                  &tempAccount.transactionCount);

        if (parsedFields == 5 && tempAccount.accountNumber == account.accountNumber) {
            // Update the account balance and transaction count
            tempAccount.balance = account.balance;
            tempAccount.transactionCount += 1;

            // Format the updated account line
            char updatedLine[256];
            snprintf(updatedLine, sizeof(updatedLine), "%d,%d,%d,%ld,%d\n",
                     tempAccount.accountNumber,
                     tempAccount.customerid,
                     tempAccount.active,
                     tempAccount.balance,
                     tempAccount.transactionCount);

            strcat(updatedContent, updatedLine);
            accountUpdated = true;
        } else {
            // Copy the line as-is if it does not match the target account
            strcat(updatedContent, line);
        }
    }

    // Rewrite the file with updated content
    freopen("acc.txt", "w", file); // Reopen in write mode to overwrite
    fputs(updatedContent, file);
    fclose(file);}
            else
            {
                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, DEPOSIT_AMOUNT_INVALID, sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));
            }
        }
        else
        {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, ACCOUNT_DEACTIVATED, sizeof(writeBuffer));
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
        write(connFD,"*Successfully updated\n", strlen("*Successfully updated\n"));
        bzero(readBuffer, sizeof(readBuffer)); // Dummy read
        read(connFD, readBuffer, sizeof(readBuffer));
        return true;

        //unlock_critical_section(&semOp);
    }
    else
    {
        // FAIL
        //unlock_critical_section(&semOp);
        return false;
    }
}

bool withdraw(int connFD) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;
    printf("Account number %d\n", account.accountNumber);

    long int withdrawAmount = 0;
    fflush(stdout);
    if (get_account_details(connFD, &account)) {
        if (account.active) {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, "&Enter the amount to withdraw:\n", sizeof(writeBuffer));
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing withdrawal prompt to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading withdrawal amount from client!");
                return false;
            }

            withdrawAmount = atol(readBuffer);
            if (withdrawAmount > 0 && withdrawAmount <= account.balance) {
                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance - withdrawAmount, 0);
                account.balance -= withdrawAmount;

                // Open the account file to update balance
                FILE *file = fopen("acc.txt", "r+");
                if (!file) {
                    perror("Could not open account file");
                    return false;
                }

                char line[256];
                char updatedContent[4096] = "";
                bool accountUpdated = false;

                while (fgets(line, sizeof(line), file)) {
                    struct Account tempAccount;
                    int parsedFields = sscanf(line, "%d,%d,%d,%ld,%d",
                                              &tempAccount.accountNumber,
                                              &tempAccount.customerid,
                                              (int*)&tempAccount.active,
                                              &tempAccount.balance,
                                              &tempAccount.transactionCount);

                    if (parsedFields == 5 && tempAccount.accountNumber == account.accountNumber) {
                        // Update the balance and transaction count
                        tempAccount.balance = account.balance;
                        tempAccount.transactionCount += 1;

                        // Format the updated account line
                        char updatedLine[256];
                        snprintf(updatedLine, sizeof(updatedLine), "%d,%d,%d,%ld,%d\n",
                                 tempAccount.accountNumber,
                                 tempAccount.customerid,
                                 tempAccount.active,
                                 tempAccount.balance,
                                 tempAccount.transactionCount);

                        strcat(updatedContent, updatedLine);
                        accountUpdated = true;
                    } else {
                        // Copy the line as-is if it does not match the target account
                        strcat(updatedContent, line);
                    }
                }

                // Rewrite the file with updated content
                freopen("acc.txt", "w", file); // Reopen in write mode to overwrite
                fputs(updatedContent, file);
                fclose(file);

                // Send success message to client
                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, "Withdrawal successful!\n", sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));
            } else {
                // Invalid withdrawal amount
                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, "*Insufficient funds or invalid amount.\n", sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));
            }
        } else {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, "*Account is deactivated.\n", sizeof(writeBuffer));
            write(connFD, writeBuffer, strlen(writeBuffer));
            return false;
        }
        write(connFD,"*Successfully withdrawn \n",strlen("*Successfully withdrawn \n"));
        bzero(readBuffer, sizeof(readBuffer)); // Dummy read to synchronize
        read(connFD, readBuffer, sizeof(readBuffer));
        return true;
    } else {
        // Failed to get account details
        return false;
    }
}

bool get_balance(int connFD) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;
    struct Account account;
    account.accountNumber = loggedInCustomer.account;  // Assuming logged-in customer data
    bool accountFound = false;

    // Open the file in read mode
    FILE *file = fopen("acc.txt", "r");
    if (!file) {
        perror("Could not open account file");
        return false;
    }

    // Read file line by line to find the matching account
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        struct Account tempAccount;
        
        // Parse CSV data
        int parsedFields = sscanf(line, "%d,%d,%d,%ld,%d",
                                  &tempAccount.accountNumber,
                                  &tempAccount.customerid,
                                  (int*)&tempAccount.active,
                                  &tempAccount.balance,
                                  &tempAccount.transactionCount);

        if (parsedFields == 5 && tempAccount.accountNumber == account.accountNumber) {
            account = tempAccount;
            accountFound = true;
            break;
        }
    }
    fclose(file);

    if (!accountFound) {
        // Account not found message
        //snprintf(writeBuffer, sizeof(writeBuffer), ACCOUNT_NOT_FOUND);
        write(connFD,"*Account not found\n", strlen("*Account not found\n"));
        return false;
    }

    // Check if the account is active
    if (!account.active) {
        snprintf(writeBuffer, sizeof(writeBuffer), ACCOUNT_DEACTIVATED);
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Send the balance to the client
    snprintf(writeBuffer, sizeof(writeBuffer), "*Your current balance is: %ld\n", account.balance);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing balance to client!");
        return false;
    }

    

    return true;
}



// bool transfer_funds(int connFD) {
//     char readBuffer[1000], writeBuffer[1000];
//     ssize_t readBytes, writeBytes;

//     struct Account senderAccount, receiverAccount;
//     senderAccount.accountNumber = loggedInCustomer.account;

//     long int transferAmount = 0;
//     long int receiverAccountNumber;

//     // Lock the critical section
//     struct sembuf semOp;
//     //lock_critical_section(&semOp);

//     // Get sender account details
//     if (!get_account_details(senderAccount.accountNumber, &senderAccount)) {
//         //unlock_critical_section(&semOp);
//         return false; // Failed to retrieve sender account details
//     }

//     if (!senderAccount.active) {
//         snprintf(writeBuffer, sizeof(writeBuffer), ACCOUNT_DEACTIVATED);
//         write(connFD, writeBuffer, strlen(writeBuffer));
//         //unlock_critical_section(&semOp);
//         return false; // Sender account is inactive
//     }

//     // Prompt for receiver account number
//     snprintf(writeBuffer, sizeof(writeBuffer), "&Enter receiver account number:\n");
//     write(connFD, writeBuffer, strlen(writeBuffer));

//     bzero(readBuffer, sizeof(readBuffer));
//     readBytes = read(connFD, readBuffer, sizeof(readBuffer));
//     if (readBytes == -1) {
//         perror("Error reading receiver account number from client!");
//         //unlock_critical_section(&semOp);
//         return false;
//     }
//     receiverAccountNumber = atol(readBuffer);

//     // Check if the receiver account exists
//     if (!get_account_details(receiverAccountNumber, &receiverAccount)) {
//         snprintf(writeBuffer, sizeof(writeBuffer), ACCOUNT_ID_DOESNT_EXIT);
//         write(connFD, writeBuffer, strlen(writeBuffer));
//         //unlock_critical_section(&semOp);
//         return false;
//     }

//     // Check if sender and receiver accounts are the same
//     if (senderAccount.accountNumber == receiverAccount.accountNumber) {
//         snprintf(writeBuffer, sizeof(writeBuffer), "*Cannot transfer funds to the same account.\n");
//         write(connFD, writeBuffer, strlen(writeBuffer));
//         //unlock_critical_section(&semOp);
//         return false;
//     }

//     // Prompt for transfer amount
//     snprintf(writeBuffer, sizeof(writeBuffer), "&Enter transfer amount:\n");
//     write(connFD, writeBuffer, strlen(writeBuffer));

//     bzero(readBuffer, sizeof(readBuffer));
//     readBytes = read(connFD, readBuffer, sizeof(readBuffer));
//     if (readBytes == -1) {
//         perror("Error reading transfer amount from client!");
//         //unlock_critical_section(&semOp);
//         return false;
//     }
//     transferAmount = atol(readBuffer);

//     // Validate transfer amount
//     if (transferAmount <= 0 || transferAmount > senderAccount.balance) {
//         //snprintf(writeBuffer, sizeof(writeBuffer), TRANSFER_AMOUNT_INVALID);
//         write(connFD,"*Invalid transfer amount\n", strlen("*Invalid transfer amount\n"));
//         //unlock_critical_section(&semOp);
//         return false;
//     }

//     // Update sender and receiver balances
//     senderAccount.balance -= transferAmount;
//     receiverAccount.balance += transferAmount;

//     // Log transactions
//     int senderTransactionID = write_transaction_to_file(senderAccount.accountNumber, senderAccount.balance + transferAmount, senderAccount.balance, 0);
//     int receiverTransactionID = write_transaction_to_file(receiverAccount.accountNumber, receiverAccount.balance - transferAmount, receiverAccount.balance, 1);

//     // Update account details in acc.txt
//     FILE *file = fopen("acc.txt", "r+");
//     if (!file) {
//         perror("Error opening account file");
//         unlock_critical_section(&semOp);
//         return false;
//     }

//     char line[256];
//     char updatedContent[4096] = "";
//     bool senderUpdated = false, receiverUpdated = false;

//     while (fgets(line, sizeof(line), file)) {
//         struct Account tempAccount;
//         sscanf(line, "%d,%d,%d,%ld,%d",
//                &tempAccount.accountNumber,
//                &tempAccount.customerid,
//                (int *)&tempAccount.active,
//                &tempAccount.balance,
//                &tempAccount.transactionCount);

//         if (tempAccount.accountNumber == senderAccount.accountNumber) {
//             tempAccount.balance = senderAccount.balance;
//             tempAccount.transactionCount++;
//             senderUpdated = true;
//         } else if (tempAccount.accountNumber == receiverAccount.accountNumber) {
//             tempAccount.balance = receiverAccount.balance;
//             tempAccount.transactionCount++;
//             receiverUpdated = true;
//         }

//         char updatedLine[256];
//         snprintf(updatedLine, sizeof(updatedLine), "%d,%d,%d,%ld,%d\n",
//                  tempAccount.accountNumber,
//                  tempAccount.customerid,
//                  tempAccount.active,
//                  tempAccount.balance,
//                  tempAccount.transactionCount);
//         strcat(updatedContent, updatedLine);
//     }

//     // Reopen in write mode to overwrite with updated content
//     freopen("acc.txt", "w", file);
//     fputs(updatedContent, file);
//     fclose(file);

//     if (!senderUpdated || !receiverUpdated) {
//         perror("Error updating accounts in file");
//         //unlock_critical_section(&semOp);
//         return false;
//     }

//     // Send success message to client
//     //snprintf(writeBuffer, sizeof(writeBuffer), TRANSFER_AMOUNT_SUCCESS);
//     write(connFD,"*Successfull transfer amount\n", strlen("*Successfull transfer amount\n"));

//     bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
//     read(connFD, readBuffer, sizeof(readBuffer));

//     //unlock_critical_section(&semOp);
//     return true; // Successful fund transfer
// }

bool change_password(int connFD) {
    char readBuffer[1000];
    ssize_t readBytes;

    // Check if account is active
    if (!loggedInCustomer.active) {
        write(connFD, "Account is deactivated.\n", 24);
        return false; // Account is inactive
    }

    // Prompt for old password
    write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading old password from client!");
        return false;
    }

    // Validate old password
    if (strcmp(readBuffer, loggedInCustomer.password) != 0) {
        write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
        return false; // Old password is incorrect
    }

    // Prompt for new password
    write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading new password from client!");
        return false;
    }
    char newPassword[30]; // Adjusted for new password size
    strncpy(newPassword, readBuffer, sizeof(newPassword)); // Copy new password

    // Prompt to re-enter new password
    write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading re-entered new password from client!");
        return false;
    }

    // Validate new password match
    if (strcmp(newPassword, readBuffer) != 0) {
        write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
        return false; // New passwords do not match
    }

    // Update account password
    strncpy(loggedInCustomer.password, newPassword, sizeof(loggedInCustomer.password)); // Update the password in the account struct

    // Open custom.txt file to update password
    FILE *file = fopen("custom.txt", "r+");
    if (!file) {
        perror("Error opening customer file");
        return false;
    }

    // Temporary storage for modified customer data
    struct Customer customer;
    char line[256];
    bool customerFound = false;
    
    // Create a temporary file to store updated customer data
    FILE *tempFile = fopen("temp_custom.txt", "w");
    if (!tempFile) {
        perror("Error creating temporary file");
        fclose(file);
        return false;
    }

    // Read through the original file, update the password if matching customer found
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%[^,],%c,%d,%[^,],%d,%d", 
               &customer.id, customer.name, &customer.gender, 
               &customer.age, customer.password, 
               &customer.account, &customer.active);

        // Check if this is the logged-in customer
        if (customer.id == loggedInCustomer.id) {
            // Update password in the structure
            strncpy(customer.password, newPassword, sizeof(customer.password));
            customerFound = true;
        }

        // Write (updated or original) customer record to temporary file
        fprintf(tempFile, "%d,%s,%c,%d,%s,%d,%d\n", 
                customer.id, customer.name, customer.gender, 
                customer.age, customer.password, 
                customer.account, customer.active);
    }

    fclose(file); // Close original file
    fclose(tempFile); // Close temporary file

    // Replace original file with updated temporary file
    remove(CUSTOMER_FILE); // Delete original file
    rename("temp_custom.txt","custom.txt"); // Rename temporary file to original name

    if (customerFound) {
        // Send success message to client
        write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));
    } else {
        write(connFD, "Customer not found.\n", 20);
    }

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    return true; // Password change was successful
}

bool view_transaction_history(int connFD) {
    char readBuffer[1000];
    ssize_t readBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    // Get current account details
    if (!get_account_details(account.accountNumber, &account)) {
        return false; // Failed to retrieve account details
    }

    if (!account.active) {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        return false; // Account is inactive
    }

    // Check if there are transactions
    if (account.transactionCount == 0) {
        write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
        return false; // No transactions found
    }

    // Send the header for transaction history
    //write(connFD, "*Transaction History:\n", 23);

    // Open trans.txt file to read transaction details
    FILE *file = fopen("trans.txt", "r");
    if (!file) {
        perror("Error opening transaction file");
        return false;
    }

    // Read each line in trans.txt and parse transaction details
    char line[256];
    memset(readBuffer,'\0',sizeof(readBuffer));

    while (fgets(line, sizeof(line), file)) {
        printf("HEllo\n");
        struct Transaction transaction;
        sscanf(line, "%d,%d,%d,%ld,%ld,%ld",
               &transaction.transactionID,
               &transaction.accountNumber,
               (int*)&transaction.operation,
               &transaction.oldBalance,
               &transaction.newBalance,
               &transaction.transactionTime);

        // Check if the transaction belongs to the current account
        if (transaction.accountNumber == account.accountNumber) {
            snprintf(readBuffer, sizeof(readBuffer),
                     "*Transaction ID: %d | Old Balance: %ld | New Balance: %ld | Type: %s\n",
                     transaction.transactionID,
                     transaction.oldBalance,
                     transaction.newBalance,
                     transaction.operation == 1 ? "Deposit" : "Withdrawal");
            

        }
    }
    fflush(stdout);
    fflush(stdin);
    write(connFD, readBuffer, strlen(readBuffer));// Send transaction info to client
    printf("REAd %s\n",readBuffer); 

    fclose(file);  // Close the transaction file

   
    return true; // Transaction history viewed successfully
}

bool loan(int connFD) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Loan loanEntry;
    loanEntry.loanID = generate_loan_id();
    loanEntry.customerID = loggedInCustomer.id;
    loanEntry.accountNumber = loggedInCustomer.account;
    loanEntry.isAssigned = 0; // Initially, loan is not assigned

    // Prompt for Loan Amount
    snprintf(writeBuffer, sizeof(writeBuffer), "&Enter the loan amount you are requesting:\n");
    write(connFD, writeBuffer, strlen(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes <= 0) {
        perror("Error reading loan amount");
        return false;
    }
    loanEntry.loanAmount = atol(readBuffer); // Convert the entered amount to a long int

    // Validate loan amount
    if (loanEntry.loanAmount <= 0) {
        snprintf(writeBuffer, sizeof(writeBuffer), "*Invalid loan amount.\n");
        write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Prompt for Loan Purpose
    snprintf(writeBuffer, sizeof(writeBuffer), "&Enter the purpose of the loan (max %d characters):\n", LOAN_PURPOSE_MAX_LEN);
    write(connFD, writeBuffer, strlen(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes <= 0) {
        perror("Error reading loan purpose");
        return false;
    }
    strncpy(loanEntry.loanPurpose, readBuffer, LOAN_PURPOSE_MAX_LEN - 1);
    loanEntry.loanPurpose[LOAN_PURPOSE_MAX_LEN - 1] = '\0'; // Ensure null termination

    // Open the loan file in append mode to record the new loan application
    FILE *file = fopen("rloan.txt", "a");
    if (!file) {
        perror("Error opening loan file");
        return false;
    }

    // Write loan data to the file in CSV format
    fprintf(file, "%d,%d,%ld,%d,%d,%s\n",
            loanEntry.loanID,
            loanEntry.accountNumber,
            loanEntry.loanAmount,
            loanEntry.customerID,
            loanEntry.isAssigned,
            loanEntry.loanPurpose);

    fclose(file);

    // Send success message to the client
    snprintf(writeBuffer, sizeof(writeBuffer), "Loan application submitted successfully.\n");
    write(connFD, writeBuffer, strlen(writeBuffer));

    return true; // Successfully applied for loan
}
int generate_loan_id() {
    static int loanID = 0;
    return ++loanID;
}
int generate_feedback_id() {
    static int id = 0;
    return ++id;
}

bool feedback(int connFD) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Feedback feedbackEntry;
    feedbackEntry.feedbackID = generate_feedback_id();
    feedbackEntry.feedbackDate = time(NULL); // Set feedback date to current time

    // Set customer ID and account number from the global logged-in customer
    feedbackEntry.customerID = loggedInCustomer.id;
    feedbackEntry.accountNumber = loggedInCustomer.account;

    // Prompt for and read Feedback Text
    snprintf(writeBuffer, sizeof(writeBuffer), "&Enter Feedback Text:\n");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending feedback text prompt");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes <= 0) {
        perror("Error reading feedback text");
        return false;
    }
    strncpy(feedbackEntry.feedbackText, readBuffer, sizeof(feedbackEntry.feedbackText) - 1);
    feedbackEntry.feedbackText[sizeof(feedbackEntry.feedbackText) - 1] = '\0'; // Ensure null termination

    // Open the feedback file in append mode
    FILE *file = fopen("feed.txt", "a");
    if (!file) {
        perror("Error opening feedback file");
        return false;
    }

    // Write feedback data to the file in CSV format
    fprintf(file, "%d,%d,%d,%s,%ld\n",
            feedbackEntry.customerID,
            feedbackEntry.feedbackID,
            feedbackEntry.accountNumber,
            feedbackEntry.feedbackText,
            feedbackEntry.feedbackDate);

    fclose(file);

    // Send success message to client
    snprintf(writeBuffer, sizeof(writeBuffer), "*Feedback submitted successfully.\n");
    write(connFD, writeBuffer, strlen(writeBuffer));

    return true; // Successfully submitted feedback
}

int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation) {
    struct Transaction newTransaction;
    newTransaction.accountNumber = accountNumber;
    newTransaction.oldBalance = oldBalance;
    newTransaction.newBalance = newBalance;
    newTransaction.operation = operation;
    newTransaction.transactionTime = time(NULL);

    // Open file for appending
    FILE *file = fopen("trans.txt", "a+");
    if (!file) {
        perror("Could not open transaction file");
        return -1;
    }

    // Find the most recent transaction ID by reading the last line
    char line[256];
    int lastTransactionID = -1;
    while (fgets(line, sizeof(line), file)) {
        int tempID;
        if (sscanf(line, "%d,", &tempID) == 1) {
            lastTransactionID = tempID;
        }
    }
    newTransaction.transactionID = lastTransactionID + 1;

    // Format and write the new transaction as a comma-separated line
    struct tm *tm_info = localtime(&newTransaction.transactionTime);
    char dateBuffer[30];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(file, "%d,%d,%d,%ld,%ld,%s\n",
            newTransaction.transactionID,
            newTransaction.accountNumber,
            newTransaction.operation,
            newTransaction.oldBalance,
            newTransaction.newBalance,
            dateBuffer);

    fclose(file);

    return newTransaction.transactionID;
}

 #endif


