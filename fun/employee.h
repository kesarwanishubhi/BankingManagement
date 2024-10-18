#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h> 
#include <errno.h>

#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"
#include "../employee.h"



#include "../fun/constants.h"


// Function Prototypes
bool empl_handler(int connFD);
bool add_new_customer(int connFD);
bool modify_customer_details(int connFD);
//bool process_loan_application(int connFD);
bool approve_reject_loan(int connFD);
bool view_assigned_loan_applications(int connFD);
int get_next_customer_id();

//=================================================================================================================
bool empl_handler(int connFD) {
    char buffer[1024];
    char loginID[50], password[50];

    // Employee Login Process
    send(connFD, "Enter Login ID: \n", strlen("Enter Login ID: \n"), 0);
    ssize_t recvBytes = recv(connFD, loginID, sizeof(loginID) - 1, 0); // Receive login ID
    if (recvBytes <= 0) {
        printf("Error receiving login ID.\n");
        return false;
    }
    loginID[recvBytes] = '\0';  // Null-terminate the login ID
    loginID[strcspn(loginID, "\n")] = 0; // Remove newline character if any
    printf("Received Login ID: %s\n", loginID);  // Debugging line

    send(connFD, "Enter Password: \n", strlen("Enter Password: \n"), 0);
    recvBytes = recv(connFD, password, sizeof(password) - 1, 0);  // Receive password
    if (recvBytes <= 0) {
        printf("Error receiving password.\n");
        return false;
    }
    password[recvBytes] = '\0';  // Null-terminate the password
    password[strcspn(password, "\n")] = 0; // Remove newline character if any
    printf("Received Password: %s\n", password);  // Debugging line

    // Check login credentials
    if (strcmp(loginID, EMPLOYEE_LOGIN_ID) != 0 || strcmp(password, EMPLOYEE_PASSWORD) != 0) {
        send(connFD, "INFO:Invalid Login ID or Password\n", strlen("INFO:Invalid Login ID or Password\n"), 0);
        printf("LOGIN message sent");
        return false;
    }

    send(connFD, "INFO:Login Successful!##\n", strlen("INFO:Login Successful!##\n"), 0);
    
    bool logged_in = true;
    while (logged_in) {
        // memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        bzero(buffer, sizeof(buffer));
        printf("HEllo \n");

        // Menu Options
        const char *menu =
            "\n=== Employee Menu ===\n"
            "1. Add New Customer\n"
            "2. Modify Customer Details\n"
            "3. Approve/Reject Loans\n"
            "4. View Assigned Loan Applications\n"
            "5. Change Password\n"
            "6. Logout\n"
            "7. Exit\n"
            "Enter your choice: ##\n";
        
        send(connFD, menu, strlen(menu), 0);
        printf("SENT MEnu \n");
        recvBytes = recv(connFD, buffer, sizeof(buffer) - 1, 0);  // Receive user's choice
        printf("Received");
        if (recvBytes <= 0) {
            printf("Error receiving choice.\n");
            break;
        }
        buffer[recvBytes] = '\0';  // Null-terminate the input
        int choice = atoi(buffer);

        switch (choice) {
            case 1:
                send(connFD, "INFO:Add New Customer selected.##\n", strlen("INFO:Add New Customer selected.##\n"), 0);
                printf("MEssage sent \n");
                add_new_customer(connFD);
                break;
            case 2:
                send(connFD, "INFO:Modify Customer Details selected.\n", strlen("INFO:Modify Customer Details selected.\n"), 0);
                printf("MEssage sent \n");
                //modify_customer_details(connFD);
                break;
            case 3:
                send(connFD, "INFO:Approve/Reject Loans selected.\n", strlen("INFO:Approve/Reject Loans selected.\n"), 0);
                //approve_reject_loan(connFD);
                printf("MEssage sent \n");
                break;
            case 4:
                send(connFD, "INFO:View Assigned Loan Applications selected.\n", strlen("INFO:View Assigned Loan Applications selected.\n"), 0);
                //view_assigned_loan_applications(connFD);
                break;
            case 5:
                send(connFD, "INFO:Change Password selected.\n", strlen("INFO:Change Password selected.\n"), 0);
                // Implement change password logic here
                printf("MEssage sent \n");
                break;
            case 6:
                send(connFD, "INFO:Logging out...\n", strlen("INFO:Logging out...\n"), 0);
                logged_in = false;
                printf("MEssage sent");
                break;
            case 7:
                send(connFD, "Exiting...\n", strlen("Exiting...\n"), 0);
                logged_in = false;
                break;
            default:
                send(connFD, "INFO:Invalid choice. Please try again.\n", strlen("INFO:Invalid choice. Please try again.\n"), 0);
                printf("MEssage sent invalid \n");
                break;
        }
    }

    return true;
}


bool add_new_customer(int connFD) {
    struct Customer newCustomer;
    char buffer[1000];
    ssize_t writeBytes;

    // Get the next available customer ID
    newCustomer.id = get_next_customer_id();

    // Prompt for customer details
    send(connFD, "Enter Customer Name: ##\n", strlen("Enter Customer Name: ##\n"), 0);
    printf("Sent \n");
    read(connFD, newCustomer.name, sizeof(newCustomer.name));
    printf("RE \n");
    newCustomer.name[strcspn(newCustomer.name, "\n")] = 0; // Remove newline

    send(connFD, "Enter Customer Gender (M/F/O): \n", strlen("Enter Customer Gender (M/F/O): \n"), 0);
    printf("Sent \n");
    recv(connFD, &newCustomer.gender, sizeof(newCustomer.gender), 0);
    printf("RE \n");

    send(connFD, "Enter Customer Age: \n", strlen("Enter Customer Age: \n"), 0);
    printf("Sent \n");
    recv(connFD, buffer, sizeof(buffer), 0);
    printf("RE \n");
    newCustomer.age = atoi(buffer);

    send(connFD, "Enter Customer Account Number: \n", strlen("Enter Customer Account Number: \n"), 0);
    printf("Sent \n");
    recv(connFD, buffer, sizeof(buffer), 0);
    printf("RE \n");
    newCustomer.account = atoi(buffer);

    // Default password setup
    snprintf(newCustomer.password, sizeof(newCustomer.password), "%s-%d", newCustomer.name, newCustomer.id);
    newCustomer.active = 1;

    // Write new customer to file
    int customerFileFD = open(CUSTOMER_FILE, O_WRONLY | O_APPEND);
    if (customerFileFD == -1) {
        perror("Error opening customer file for writing!");
        return false;
    }

    if (write(customerFileFD, &newCustomer, sizeof(struct Customer)) == -1) {
        perror("Error writing new customer to file!");
        close(customerFileFD);
        return false;
    }

    close(customerFileFD);

    // Notify the client of success and return customer ID, account number, and password
    send(connFD, ADD_NEW_CUSTOMER_SUCCESS, strlen(ADD_NEW_CUSTOMER_SUCCESS), 0);
    printf("Sent SUCCESS \n");

    // Prepare response with customer ID, account number, and password
    snprintf(buffer, sizeof(buffer), "INFO:Customer ID: %d\nAccount Number: %d\nPassword: %s\n", 
             newCustomer.id, newCustomer.account, newCustomer.password);
    send(connFD, buffer, strlen(buffer), 0);

    return true;
}


bool modify_customer_details(int connFD) {
    int customerID;
    struct Customer updatedCustomer;
    char buffer[1000];

    send(connFD, "Enter Customer ID to modify: ", strlen("Enter Customer ID to modify: "), 0);
    recv(connFD, buffer, sizeof(buffer), 0);
    customerID = atoi(buffer);

    int customerFileFD = open(CUSTOMER_FILE, O_RDWR);
    if (customerFileFD == -1) {
        perror("Error opening customer file for reading/writing!");
        return false;
    }

    off_t offset = lseek(customerFileFD, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1) {
        perror("Customer record not found!");
        close(customerFileFD);
        return false;
    }

    if (read(customerFileFD, &updatedCustomer, sizeof(struct Customer)) == -1) {
        perror("Error reading customer record!");
        close(customerFileFD);
        return false;
    }

    // Modify customer details
    send(connFD, "Enter new Customer Name (leave blank for no change):", strlen("Enter new Customer Name (leave blank for no change):"), 0);
    recv(connFD, buffer, sizeof(buffer), 0);
    if (strlen(buffer) > 0) {
        strcpy(updatedCustomer.name, buffer);
    }

    send(connFD, "Enter new Customer Gender (leave blank for no change): ", strlen("Enter new Customer Gender (leave blank for no change): "), 0);
    recv(connFD, buffer, sizeof(buffer), 0);
    if (strlen(buffer) > 0) {
        updatedCustomer.gender = buffer[0];
    }

    send(connFD, "Enter new Customer Age (leave blank for no change): ", strlen("Enter new Customer Age (leave blank for no change): "), 0);
    recv(connFD, buffer, sizeof(buffer), 0);
    if (strlen(buffer) > 0) {
        updatedCustomer.age = atoi(buffer);
    }

    // Seek to the correct position to update the record
    lseek(customerFileFD, offset, SEEK_SET);
    if (write(customerFileFD, &updatedCustomer, sizeof(struct Customer)) == -1) {
        perror("Error updating customer record!");
        close(customerFileFD);
        return false;
    }

    close(customerFileFD);
    send(connFD, MODIFY_CUSTOMER_SUCCESS, strlen(MODIFY_CUSTOMER_SUCCESS), 0);
    return true;
}
/*bool process_loan_application(int connFD) {
    // Similar structure to add_new_customer but for loan applications
    struct Loan newLoanApplication;
    char buffer[1000];
    ssize_t writeBytes;

    // Prompt for loan application details
    write(connFD, "Enter Customer ID for Loan Application: ", 40);
    read(connFD, buffer, sizeof(buffer));
    newLoanApplication.customerID = atoi(buffer);

    write(connFD, "Enter Loan Amount: ", 19);
    read(connFD, buffer, sizeof(buffer));
    newLoanApplication.amount = atoi(buffer);

    write(connFD, "Enter Loan Purpose: ", 20);
    read(connFD, newLoanApplication.purpose, sizeof(newLoanApplication.purpose));

    // Write new loan application to file
    int loanFileFD = open(LOAN_APPLICATION_FILE, O_WRONLY | O_APPEND);
    if (loanFileFD == -1) {
        perror("Error opening loan application file for writing!");
        return false;
    }

    if (write(loanFileFD, &newLoanApplication, sizeof(struct LoanApplication)) == -1) {
        perror("Error writing new loan application to file!");
        close(loanFileFD);
        return false;
    }

    close(loanFileFD);
    write(connFD, LOAN_APPLICATION_RECEIVED, strlen(LOAN_APPLICATION_RECEIVED));
    return true;
}*/


bool approve_reject_loan(int connFD) {
    struct Loan loan;
    int loanID;
    char decision[10];
    ssize_t writeBytes;
    int loanFileFD;
    bool found = false;

    // Request the loan ID from the client
    writeBytes = write(connFD, "Enter Loan Application ID to approve/reject: ", 44);
    read(connFD, decision, sizeof(decision));
    loanID = atoi(decision);

    // Open the loan file to read and update
    loanFileFD = open(LOAN_FILE, O_RDWR);
    if (loanFileFD == -1) {
        perror("Error opening loan file!");
        return false;
    }

    // Search for the loan record with the given loan ID
    while (read(loanFileFD, &loan, sizeof(struct Loan)) > 0) {
        if (loan.loanID == loanID) {
            found = true;
            break;
        }
    }

    if (!found) {
        // Loan ID not found
        write(connFD, "Loan ID not found.\n", strlen("Loan ID not found.\n"));
        close(loanFileFD);
        return false;
    }

    // Get the employee's decision (approve/reject)
    writeBytes = write(connFD, "Enter employee decision (approve/reject): ", 42);
    read(connFD, decision, sizeof(decision));

    // Update the loan status based on the decision
    if (strcmp(decision, "approve") == 0) {
        strcpy(loan.status, "approved");
        write(connFD, LOAN_APPROVED, strlen(LOAN_APPROVED));
    } else if (strcmp(decision, "reject") == 0) {
        strcpy(loan.status, "rejected");
        write(connFD, LOAN_REJECTED, strlen(LOAN_REJECTED));
    } else {
        // Invalid decision
        write(connFD, "Invalid decision entered. Please try again.\n", 45);
        close(loanFileFD);
        return false;
    }

    // Move the file pointer back to the location of the loan record to overwrite it
    lseek(loanFileFD, -sizeof(struct Loan), SEEK_CUR);

    // Write the updated loan record back to the file
    if (write(loanFileFD, &loan, sizeof(struct Loan)) == -1) {
        perror("Error writing updated loan record!");
        close(loanFileFD);
        return false;
    }

    // Close the file
    close(loanFileFD);

    return true;
}


bool view_assigned_loan_applications(int connFD) {
    // Read and display assigned loan applications from the loan application file
    int loanFileFD = open(LOAN_FILE, O_RDONLY);
    if (loanFileFD == -1) {
        perror("Error opening loan application file for reading!");
        return false;
    }

    struct Loan loanApplication;
    char writeBuffer[10000] = {0};
    ssize_t readBytes;

    // Iterate through the loan applications
    while ((readBytes = read(loanFileFD, &loanApplication, sizeof(struct Loan))) > 0) {
        // Append loan application details to writeBuffer
        char tempBuffer[1000];
       
        sprintf(tempBuffer, "Loan Application ID: %d, Customer ID: %d, Amount: %ld, Purpose: %s\nStatus: %s\n",
                loanApplication.loanID, loanApplication.customerID, loanApplication.loanAmount, loanApplication.loanPurpose, loanApplication.status);
        strcat(writeBuffer, tempBuffer);
    }

    if (strlen(writeBuffer) == 0) {
        send(connFD, NO_LOAN_APPLICATIONS, strlen(NO_LOAN_APPLICATIONS), 0);
    } else {
        send(connFD, VIEW_LOAN_APPLICATIONS, strlen(VIEW_LOAN_APPLICATIONS), 0);
        send(connFD, writeBuffer, strlen(writeBuffer), 0);
    }

    close(loanFileFD);
    return true;
}
int get_next_customer_id(int connFD) {
    int maxID = 0;
    struct Customer customer;
    int customerFileFD = open(CUSTOMER_FILE, O_RDONLY);

    if (customerFileFD == -1) {
        perror("Error opening customer file for reading!");
        send(connFD, "Error opening customer file for reading!\n", 42, 0);
        return -1; // or handle as needed
    }

    // Read through the customer file to find the max ID
    while (read(customerFileFD, &customer, sizeof(struct Customer)) > 0) {
        if (customer.id > maxID) {
            maxID = customer.id;
        }
    }

    close(customerFileFD);

    // Prepare the next customer ID to send back
    char response[100];
    snprintf(response, sizeof(response), "Next Customer ID: %d\n", maxID + 1);
    send(connFD, response, strlen(response), 0);

    return maxID + 1; // Return the next available ID
}


#endif