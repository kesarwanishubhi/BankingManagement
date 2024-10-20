#ifndef CUSTOMER_H
#define CUSTOMER_H



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
int generate_unique_loan_id();
int log_loan_application(int accountNumber, long int loanAmount, const char *loanPurpose);

// =====================================================

// Function Definition =================================


bool customer_operation_handler(int connFD)
{
    if (login_handler(connFD, &loggedInCustomer))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client

        // Get a semaphore for the user
        key_t semKey = ftok(CUSTOMER_FILE, loggedInCustomer.account); // Generate a key based on the account number hence, different customers will have different semaphores

        union semun
        {
            int val; // Value of the semaphore
        } semSet;

        int semctlStatus;
        semIdentifier = semget(semKey, 1, 0); // Get the semaphore if it exists
        if (semIdentifier == -1)
        {
            semIdentifier = semget(semKey, 1, IPC_CREAT | 0700); // Create a new semaphore
            if (semIdentifier == -1)
            {
                perror("Error while creating semaphore!");
                _exit(1);
            }

            semSet.val = 1; // Set a binary semaphore
            semctlStatus = semctl(semIdentifier, 0, SETVAL, semSet);
            if (semctlStatus == -1)
            {
                perror("Error while initializing a binary sempahore!");
                _exit(1);
            }
        }

        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_LOGIN_SUCCESS);
        bool logged_in = true;
        while (logged_in)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, CUSTOMER_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
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
            
            // printf("READ BUFFER : %s\n", readBuffer);
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
                get_transaction_detail(connFD, loggedInCustomer.account);
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

    long int depositAmount = 0;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

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
                unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading deposit money from client!");
                unlock_critical_section(&semOp);
                return false;
            }

            depositAmount = atol(readBuffer);
            if (depositAmount != 0)
            {
                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance + depositAmount, 1);
                write_transaction_to_array(account.transactions, newTransactionID);
                account.transactionCount++;

                account.balance += depositAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
                off_t offset = lseek(accountFileDescriptor, account.accountNumber * sizeof(struct Account), SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1)
                {
                    perror("Error obtaining write lock on account file!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1)
                {
                    perror("Error storing updated deposit money in account record!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, DEPOSIT_AMOUNT_SUCCESS, sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));

                bzero(readBuffer, sizeof(readBuffer)); // Dummy read
                read(connFD, readBuffer, sizeof(readBuffer));

                get_balance(connFD);

                unlock_critical_section(&semOp);
                return true;
            }
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

        bzero(readBuffer, sizeof(readBuffer)); // Dummy read
        read(connFD, readBuffer, sizeof(readBuffer));

        unlock_critical_section(&semOp);
    }
    else
    {
        // FAIL
        unlock_critical_section(&semOp);
        return false;
    }
}

bool withdraw(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    long int withdrawalAmount = 0;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    if (get_account_details(connFD, &account))
    {
        if (account.active)
        {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, WITHDRAW_AMOUNT, sizeof(writeBuffer));
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing WITHDRAW_AMOUNT to client!");
                unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading withdrawal money from client!");
                unlock_critical_section(&semOp);
                return false;
            }

            withdrawalAmount = atol(readBuffer);
            if (withdrawalAmount > 0 && withdrawalAmount <= account.balance)
            {
                int newTransactionID = write_transaction_to_file(account.accountNumber, account.balance, account.balance - withdrawalAmount, 0);
                write_transaction_to_array(account.transactions, newTransactionID);
                account.transactionCount++;

                account.balance -= withdrawalAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
                off_t offset = lseek(accountFileDescriptor, account.accountNumber * sizeof(struct Account), SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1)
                {
                    perror("Error obtaining write lock on account file!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1)
                {
                    perror("Error storing updated withdrawal money in account record!");
                    unlock_critical_section(&semOp);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, WITHDRAW_AMOUNT_SUCCESS, sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));

                bzero(readBuffer, sizeof(readBuffer)); // Dummy read
                read(connFD, readBuffer, sizeof(readBuffer));

                get_balance(connFD);

                unlock_critical_section(&semOp);
                close(accountFileDescriptor);

                return true;
            }
            else if (withdrawalAmount <= 0)
            {
                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer, WITHDRAW_AMOUNT_INVALID, sizeof(writeBuffer));
                write(connFD, writeBuffer, strlen(writeBuffer));
            }
            else
            {
                bzero(writeBuffer, sizeof(writeBuffer));
                strncpy(writeBuffer,"*Enter the valid amount", sizeof("*Enter the valid amount"));
                write(connFD, writeBuffer, strlen(writeBuffer));
            }
        }
        else
        {
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, ACCOUNT_DEACTIVATED, sizeof(writeBuffer));
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
        
        bzero(readBuffer, sizeof(readBuffer)); // Dummy read
        read(connFD, readBuffer, sizeof(readBuffer));

        unlock_critical_section(&semOp);
    }
    else
    {
        // FAIL
        unlock_critical_section(&semOp);
        return false;
    }
}
bool get_balance(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Retrieve account details
    if (get_account_details(connFD, &account))
    {
        if (account.active)
        {
            // Prepare the balance message
            //snprintf(writeBuffer, sizeof(writeBuffer), "Your current balance is: %ld", account.balance);

            bzero(writeBuffer, sizeof(writeBuffer));
            snprintf(writeBuffer, sizeof(writeBuffer), "Your current balance is: %ld", account.balance);

            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing balance to client!");
                unlock_critical_section(&semOp);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer)); // Dummy read to wait for client acknowledgment
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error reading acknowledgment from client!");
                unlock_critical_section(&semOp);
                return false;
            }

            unlock_critical_section(&semOp);
            return true;
        }
        else
        {
            // Account is deactivated
            bzero(writeBuffer, sizeof(writeBuffer));
            strncpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT, sizeof(writeBuffer));
            write(connFD, writeBuffer, strlen(writeBuffer));
        }
    }
    else
    {
        // Failed to retrieve account details
        unlock_critical_section(&semOp);
        return false;
    }

    // Dummy read to wait for client acknowledgment before unlocking
    bzero(readBuffer, sizeof(readBuffer)); 
    read(connFD, readBuffer, sizeof(readBuffer)); 
    unlock_critical_section(&semOp);
    return false; // Return false if the account is inactive
}

bool transfer_funds(int connFD) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account senderAccount, receiverAccount;
    senderAccount.accountNumber = loggedInCustomer.account;

    long int transferAmount = 0;
    long int receiverAccountNumber;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Get sender account details
    if (!get_account_details(connFD, &senderAccount)) {
        unlock_critical_section(&semOp);
        return false; // Failed to retrieve sender account details
    }

    if (!senderAccount.active) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strncpy(writeBuffer, ACCOUNT_DEACTIVATED, sizeof(writeBuffer));
        write(connFD, writeBuffer, strlen(writeBuffer));
        unlock_critical_section(&semOp);
        return false; // Sender account is inactive
    }

    // Prompt for receiver account number
    bzero(writeBuffer, sizeof(writeBuffer));
    strncpy(writeBuffer, GET_ACCOUNT_NUMBER, sizeof(writeBuffer));
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading receiver account number from client!");
        unlock_critical_section(&semOp);
        return false;
    }
    receiverAccountNumber = atol(readBuffer); // Convert input to long int

    // Check if the receiver account exists
    receiverAccount.accountNumber = receiverAccountNumber;
    if (!get_account_details(connFD, &receiverAccount)) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strncpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT, sizeof(writeBuffer));
        write(connFD, writeBuffer, strlen(writeBuffer));
        unlock_critical_section(&semOp);
        return false; // Receiver account does not exist
    }

    // Check if sender and receiver accounts are the same
    if (senderAccount.accountNumber == receiverAccount.accountNumber) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strncpy(writeBuffer, "Cannot transfer funds to the same account.", sizeof(writeBuffer));
        write(connFD, writeBuffer, strlen(writeBuffer));
        unlock_critical_section(&semOp);
        return false; // Can't transfer to the same account
    }

    // Prompt for transfer amount
    bzero(writeBuffer, sizeof(writeBuffer));
    strncpy(writeBuffer, WITHDRAW_AMOUNT, sizeof(writeBuffer));
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading transfer amount from client!");
        unlock_critical_section(&semOp);
        return false;
    }
    transferAmount = atol(readBuffer); // Convert input to long int

    // Validate transfer amount
    if (transferAmount <= 0 || transferAmount > senderAccount.balance) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strncpy(writeBuffer, WITHDRAW_AMOUNT_INVALID, sizeof(writeBuffer));
        write(connFD, writeBuffer, strlen(writeBuffer));
        unlock_critical_section(&semOp);
        return false; // Invalid transfer amount
    }

    // Update sender account balance
    senderAccount.balance -= transferAmount;

    // Update receiver account balance
    receiverAccount.balance += transferAmount;

    // Log transactions
    int senderTransactionID = write_transaction_to_file(senderAccount.accountNumber, senderAccount.balance + transferAmount, senderAccount.balance, 0);
    write_transaction_to_array(senderAccount.transactions, senderTransactionID);
    senderAccount.transactionCount++;

    int receiverTransactionID = write_transaction_to_file(receiverAccount.accountNumber, receiverAccount.balance - transferAmount, receiverAccount.balance, 1);
    write_transaction_to_array(receiverAccount.transactions, receiverTransactionID);
    receiverAccount.transactionCount++;

    // Update accounts in the file
    int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
    if (accountFileDescriptor == -1) {
        perror("Error opening account file for writing!");
        unlock_critical_section(&semOp);
        return false; // Can't open account file
    }

    // Update sender account
    off_t senderOffset = lseek(accountFileDescriptor, senderAccount.accountNumber * sizeof(struct Account), SEEK_SET);
    if (senderOffset == -1 || write(accountFileDescriptor, &senderAccount, sizeof(struct Account)) != sizeof(struct Account)) {
        perror("Error writing sender account!");
        close(accountFileDescriptor);
        unlock_critical_section(&semOp);
        return false; // Failed to write sender account
    }

    // Update receiver account
    off_t receiverOffset = lseek(accountFileDescriptor, receiverAccount.accountNumber * sizeof(struct Account), SEEK_SET);
    if (receiverOffset == -1 || write(accountFileDescriptor, &receiverAccount, sizeof(struct Account)) != sizeof(struct Account)) {
        perror("Error writing receiver account!");
        close(accountFileDescriptor);
        unlock_critical_section(&semOp);
        return false; // Failed to write receiver account
    }

    close(accountFileDescriptor); // Close account file descriptor

    // Send success message to client
    bzero(writeBuffer, sizeof(writeBuffer));
    strncpy(writeBuffer, WITHDRAW_AMOUNT_SUCCESS, sizeof(writeBuffer));
    write(connFD, writeBuffer, strlen(writeBuffer));

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    unlock_critical_section(&semOp);
    return true; // Successful fund transfer
}

bool change_password(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Get current account details
    if (!loggedInCustomer.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        unlock_critical_section(&semOp);
        return false; // Account is inactive
    }

    // Prompt for old password
    write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading old password from client!");
        unlock_critical_section(&semOp);
        return false;
    }

    // Validate old password
    if (strcmp(readBuffer, loggedInCustomer.password) != 0) // Assuming account.password stores the current password
    {
        write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
        unlock_critical_section(&semOp);
        return false; // Old password is incorrect
    }

    // Prompt for new password
    write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading new password from client!");
        unlock_critical_section(&semOp);
        return false;
    }
    char newPassword[100]; // Assuming a reasonable size for the new password
    strncpy(newPassword, readBuffer, sizeof(newPassword)); // Copy new password

    // Prompt to re-enter new password
    write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading re-entered new password from client!");
        unlock_critical_section(&semOp);
        return false;
    }

    // Validate new password match
    if (strcmp(newPassword, readBuffer) != 0)
    {
        write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
        unlock_critical_section(&semOp);
        return false; // New passwords do not match
    }

    // Update account password
    strncpy(loggedInCustomer.password, newPassword, sizeof(loggedInCustomer.password)); // Update the password in the account struct

    // Open account file to update password
    int accountFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
    off_t offset = lseek(accountFileDescriptor, loggedInCustomer.account * sizeof(struct Customer), SEEK_SET);
    struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
    fcntl(accountFileDescriptor, F_SETLKW, &lock);

    // Write the updated account information back to the file
    writeBytes = write(accountFileDescriptor, &loggedInCustomer, sizeof(struct Customer));
    if (writeBytes == -1)
    {
        perror("Error storing updated password in account record!");
        unlock_critical_section(&semOp);
        fcntl(accountFileDescriptor, F_UNLCK, &lock);
        return false; // Failed to update password
    }

    // Unlock the file
    lock.l_type = F_UNLCK;
    fcntl(accountFileDescriptor, F_SETLK, &lock);

    // Send success message to client
    write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    unlock_critical_section(&semOp);
    return true; // Password change was successful
}

bool lock_critical_section(struct sembuf *semOp)
{
    semOp->sem_flg = SEM_UNDO;
    semOp->sem_op = -1;
    semOp->sem_num = 0;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while locking critical section");
        return false;
    }
    return true;
}

bool unlock_critical_section(struct sembuf *semOp)
{
    semOp->sem_op = 1;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while operating on semaphore!");
        _exit(1);
    }
    return true;
}
void write_transaction_to_array(int *transactionArray, int ID)
{
    // Check if there's any free space in the array to write the new transaction ID
    int iter = 0;
    while (transactionArray[iter] != -1)
        iter++;

    if (iter >= MAX_TRANSACTIONS)
    {
        // No space
        for (iter = 1; iter < MAX_TRANSACTIONS; iter++)
            // Shift elements one step back discarding the oldest transaction
            transactionArray[iter - 1] = transactionArray[iter];
        transactionArray[iter - 1] = ID;
    }
    else
    {
        // Space available
        transactionArray[iter] = ID;
    }
}
bool view_transaction_history(int connFD)
{
    char readBuffer[1000];
    ssize_t readBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Get current account details
    if (!get_account_details(connFD, &account))
    {
        unlock_critical_section(&semOp);
        return false; // Failed to retrieve account details
    }

    if (!account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        unlock_critical_section(&semOp);
        return false; // Account is inactive
    }

    // Check if there are transactions
    if (account.transactionCount == 0)
    {
        write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
        unlock_critical_section(&semOp);
        return false; // No transactions found
    }

    // Send the header for transaction history
    write(connFD, "Transaction History:\n", 22);

    // Iterate through transaction IDs and retrieve their details
    for (int i = 0; i < account.transactionCount; i++)
    {
        int transactionID = account.transactions[i]; // Get the transaction ID
        
        struct Transaction transaction;
        if (!get_transaction_details(transactionID, &transaction)) // Retrieve full transaction details
        {
            write(connFD, "Error retrieving transaction details.\n", 38);
            continue; // Skip this transaction if there's an error
        }

        snprintf(readBuffer, sizeof(readBuffer), 
                 "Transaction ID: %d | oldBalance: %.2ld | New Balance: %.2ld | Type: %s\n", 
                 transaction.transactionID, 
                 transaction.oldBalance, 
                 transaction.newBalance, 
                 transaction.operation == 1 ? "Deposit" : "Withdrawal");

        write(connFD, readBuffer, strlen(readBuffer)); // Send transaction info
    }

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    unlock_critical_section(&semOp);
    return true; // Transaction history viewed successfully
}


bool loan(int connFD)
{
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Get current account details
    if (!get_account_details(connFD, &account))
    {
        unlock_critical_section(&semOp);
        return false; // Failed to retrieve account details
    }

    if (!account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED)); // Account is inactive
        unlock_critical_section(&semOp);
        return false;
    }

    // Prompt for loan amount
    write(connFD, LOAN_AMOUNT_PROMPT, strlen(LOAN_AMOUNT_PROMPT)); // Enter the amount you wish to apply for the loan:
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading loan amount from client!");
        unlock_critical_section(&semOp);
        return false;
    }

    long int loanAmount = atol(readBuffer); // Convert input to long int
    if (loanAmount <= 0)
    {
        write(connFD, LOAN_AMOUNT_INVALID, strlen(LOAN_AMOUNT_INVALID)); // Invalid loan amount
        unlock_critical_section(&semOp);
        return false;
    }

    // Prompt for loan purpose
    write(connFD, LOAN_PURPOSE_PROMPT, strlen(LOAN_PURPOSE_PROMPT)); // Enter the purpose for the loan:
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading loan purpose from client!");
        unlock_critical_section(&semOp);
        return false;
    }

    char loanPurpose[256]; // Assuming a reasonable size for loan purpose
    strncpy(loanPurpose, readBuffer, sizeof(loanPurpose));

    // Here you might want to implement additional logic to check the customer's eligibility, credit score, etc.

    // Log the loan application
    int newLoanID = log_loan_application(account.accountNumber, loanAmount, loanPurpose);
    if (newLoanID == -1)
    {
        write(connFD, LOAN_APPLICATION_FAILURE, strlen(LOAN_APPLICATION_FAILURE)); // Failed to log loan application
        unlock_critical_section(&semOp);
        return false;
    }

    // Send success message to client
    snprintf(writeBuffer, sizeof(writeBuffer), "%s%d\n", LOAN_APPLICATION_SUCCESS, newLoanID);
    write(connFD, writeBuffer, strlen(writeBuffer)); // Send loan application ID

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    unlock_critical_section(&semOp);
    return true; // Loan application submitted successfully
}

int generate_unique_loan_id() {
    static int loanID = 5000;  // Simulate unique loan ID generation starting at 5000
    return loanID++;
}

// Function to log loan application details
int log_loan_application(int accountNumber, long int loanAmount, const char *loanPurpose) {
    // Open the loan file in append mode
    int file = open(LOAN_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1) {
        perror("Error opening loan file");
        return -1;  // Return -1 to indicate failure
    }

    // Generate a unique loan ID
    int loanID = generate_unique_loan_id();

    // Get current time for logging the loan application date
    time_t now = time(NULL);
    char *formattedTime = ctime(&now);  // Convert time to human-readable format (note: ctime adds a newline)

    // Prepare the loan data to write to the file
    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer),
                       "LoanID: %d\nAccountNumber: %d\nLoanAmount: %ld\nCustomerId: %d\nisAssigned: %d\nLoanPurpose: %s\nStatus: %s\nApplicationDate: %s------------------------------------\n",
                       loanID, accountNumber, loanAmount,loggedInCustomer.id,1, loanPurpose,"pending" ,formattedTime);

    // Write the loan data to the file using system call
    ssize_t writeBytes = write(file, buffer, len);
    if (writeBytes == -1) {
        perror("Error writing loan application to file");
        close(file);
        return -1;  // Return -1 to indicate failure
    }

    // Close the loan file after writing
    close(file);

    // Return the unique loan ID
    return loanID;
}

bool feedback(int connFD)
{
    char readBuffer[1000];
    ssize_t readBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    // Lock the critical section
    struct sembuf semOp;
    lock_critical_section(&semOp);

    // Get current account details
    if (!get_account_details(connFD, &account))
    {
        unlock_critical_section(&semOp);
        return false; // Failed to retrieve account details
    }

    if (!account.active)
    {
        write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED)); // Account is inactive
        unlock_critical_section(&semOp);
        return false;
    }

    // Prompt for feedback
    write(connFD, FEEDBACK_PROMPT, strlen(FEEDBACK_PROMPT)); // Please enter your feedback (max 500 characters): 
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading feedback from client!");
        unlock_critical_section(&semOp);
        return false;
    }

    if (strlen(readBuffer) > 500)
    {
        write(connFD, FEEDBACK_TOO_LONG, strlen(FEEDBACK_TOO_LONG)); // Feedback too long
        unlock_critical_section(&semOp);
        return false;
    }

    // Log the feedback
    int newFeedbackID = log_feedback_to_file(account.accountNumber, readBuffer);
    if (newFeedbackID == -1)
    {
        write(connFD, FEEDBACK_FAILURE, strlen(FEEDBACK_FAILURE)); // Failed to log feedback
        unlock_critical_section(&semOp);
        return false;
    }

    // Send success message to client
    write(connFD, FEEDBACK_SUCCESS, strlen(FEEDBACK_SUCCESS));

    bzero(readBuffer, sizeof(readBuffer)); // Dummy read for client acknowledgment
    read(connFD, readBuffer, sizeof(readBuffer));

    unlock_critical_section(&semOp);
    return true; // Feedback submitted successfully
}

int log_feedback_to_file(int accountNumber, const char *feedbackText) {
    // Open the feedback file in append mode
    int file = open(FEEDBACK_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);  
    if (file == -1) {
        perror("Error opening feedback file");
        return -1;  // Return -1 to indicate failure
    }

    // Generate a unique feedback ID
    int feedbackID = generate_unique_feedback_id();

    // Get current time for logging the feedback submission date
    time_t now = time(NULL);
    char *formattedTime = ctime(&now);  // Convert time to a human-readable format

    // Prepare the feedback data to write to the file
    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), 
                       "FeedbackID: %d\nAccountNumber: %d\nFeedback: %s\nFeedbackDate: %s------------------------------------\n", 
                       feedbackID, accountNumber, feedbackText, formattedTime);

    // Write the feedback data to the file using system call
    ssize_t writeBytes = write(file, buffer, len);
    if (writeBytes == -1) {
        perror("Error writing feedback to file");
        close(file);
        return -1;  // Return -1 to indicate failure
    }

    // Close the file after writing
    close(file);

    return feedbackID;  // Return the unique feedback ID
}
int generate_unique_feedback_id()
{
    // You can use a combination of time and a counter or any other logic.
    static int counter = 1; // A static counter to ensure uniqueness
    return (int)time(NULL) + (counter++); // Use timestamp + counter
}
/*bool logout(int connFD, bool isAdmin)
{
    char readBuffer[1000];

    
        write(connFD, CUSTOMER_LOGOUT, strlen(CUSTOMER_LOGOUT));
   

    // Dummy read to wait for acknowledgment from the client
    read(connFD, readBuffer, sizeof(readBuffer));

    // Close the connection with the client
    close(connFD);

    return true; // Successful logout
}

void exit_system(int connFD)
{
    char readBuffer[1000];

    // Inform the client that the system is exiting
    write(connFD, EXIT_MESSAGE, strlen(EXIT_MESSAGE));

    // Dummy read to wait for acknowledgment from the client
    read(connFD, readBuffer, sizeof(readBuffer));

    // Close the connection
    close(connFD);

    // Terminate the server process or handle graceful shutdown (depends on system design)
    exit(0); // Terminates the process (you can adjust this based on your application)
}*/
int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation)
{
    struct Transaction newTransaction;
    newTransaction.accountNumber = accountNumber;
    newTransaction.oldBalance = oldBalance;
    newTransaction.newBalance = newBalance;
    newTransaction.operation = operation;
    newTransaction.transactionTime = time(NULL);

    ssize_t readBytes, writeBytes;

    int transactionFileDescriptor = open(TRANSACTION_FILE, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);

    // Get most recent transaction number
    off_t offset = lseek(transactionFileDescriptor, -sizeof(struct Transaction), SEEK_END);
    if (offset >= 0)
    {
        // There exists at least one transaction record
        struct Transaction prevTransaction;
        readBytes = read(transactionFileDescriptor, &prevTransaction, sizeof(struct Transaction));

        newTransaction.transactionID = prevTransaction.transactionID + 1;
    }
    else
        // No transaction records exist
        newTransaction.transactionID = 0;

    writeBytes = write(transactionFileDescriptor, &newTransaction, sizeof(struct Transaction));

    return newTransaction.transactionID;
}


#endif


