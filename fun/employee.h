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
#include "../Assignedloan.h"
#include "../fun/constants.h"
#define BUFFER_SIZE 1024

// Function Prototypes
bool add_new_customer(int connFD);
bool empl_handler(int connFD);
bool change_p(int connFD);
bool modify_customer_details(int connFD);
//bool process_loan_application(int connFD);
void approve_reject_loan(int connFD);
void view_assigned_loan_applications(int connFD);
int get_next_customer_id();

bool empl_handler(int connFD)
{
    char buffer[1024];int id;
    
    char loginID[1024], password[1024];
    bool logged_in=false;

    // Employee Login Process
    fflush(stdout);
    fflush(stdin);
    write(connFD, "&Enter Login ID: \n", strlen("&Enter Login ID: \n"));
    memset(loginID,'\0',sizeof(loginID));
    ssize_t readBytes = read(connFD, loginID, sizeof(loginID)); // Receive login ID
    if (readBytes <= 0) {
        printf("Error receiving login ID.\n");
        return false;
    }
    // loginID[readBytes] = '\0';  // Null-terminate the login ID
    loginID[strcspn(loginID, "\n")] = '\0'; // Remove newline character if any
    id=atoi(loginID);
    printf("Received Login ID: %s\n", loginID);  // Debugging line

    write(connFD, "&Enter Password: \n", strlen("&Enter Password: \n"));
    memset(password,'\0',sizeof(password));
    readBytes = read(connFD, password, sizeof(password));  // Receive password
    if (readBytes <= 0) {
        printf("Error receiving password.\n");
        return false;
    }
    // password[readBytes] = '\0';  // Null-terminate the password
    password[strcspn(password, "\n")] = '\0'; // Remove newline character if any
    printf("Received Password: %s\n", password);  // Debugging line

   FILE *file = fopen("employee.txt", "r");
    if (!file) {
        perror("Error opening file");
        return 0;
    }

    struct Employee employee;
    //char buffer[BUFFER_SIZE];
    int found = 0;

    // Read each line in employee.txt and parse it
    while (fgets(buffer, BUFFER_SIZE, file)) {
        // Parse the line based on the CSV structure: name,id,type,password
        sscanf(buffer, "%49[^,],%d,%d,%49[^,]",
               employee.name, &employee.id, &employee.type, employee.password);

        // Check if both ID and password match
        if (employee.id == id ) {
            found = 1;
            logged_in=true;
            break;
        }
    }

    fclose(file);

    if (!found) {
        write(connFD,"*Invalid credentials \n",sizeof("*Invalid credentials \n"));
        return false;
    }
    
    



    //write(connFD, "*Login Successful!\n", strlen("*Login Successful!\n"));
    
    
    while (logged_in) {
        // memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        bzero(buffer, sizeof(buffer));
        printf("HEllo \n");
        fflush(stdout);
        // Menu Options
        const char *menu =
            "& Login successful\n"
            "=== Employee Menu ===\n"
            "1. Add New Customer\n"
            "2. Modify Customer Details\n"
            "3. Approve/Reject Loans\n"
            "4. View Assigned Loan Applications\n"
            "5. Change Password\n"
            "6. Logout\n"
            "7. Exit\n"
            "Enter your choice:\n";
        
        write(connFD, menu, strlen(menu));
        //printf("SENT MEnu \n");
        memset(buffer,'\0',sizeof(buffer));
        readBytes = read(connFD, buffer, sizeof(buffer));  // Receive user's choice
        //printf("Received");
        if (readBytes <= 0) {
            printf("Error receiving choice.\n");
            break;
        }  // Null-terminate the input
        int choice = atoi(buffer);

        switch (choice) {
            case 1:
                
                add_new_customer(connFD);
                break;
            case 2:
                
                modify_customer_details(connFD);
                break;
            case 3:
                //write(connFD, "*Approve/Reject Loans selected.\n", strlen("*Approve/Reject Loans selected.\n"));
                approve_reject_loan(connFD);
                //printf("MEssage sent \n");
                break;
            case 4:
                //write(connFD, "*View Assigned Loan Applications selected.\n", strlen("*View Assigned Loan Applications selected.\n"));
                view_assigned_loan_applications(connFD);
                break;
            case 5:
                change_p(connFD);
                break;
            case 6:
                write(connFD, "*Logging out...\n", strlen("*Logging out...\n"));
                logged_in = false;
                //printf("MEssage sent");
                break;
            case 7:
                write(connFD, "*Exiting...\n", strlen("*Exiting...\n"));
                logged_in = false;
                break;
            default:
                write(connFD, "*Invalid choice. Please try again.\n", strlen("*Invalid choice. Please try again.\n"));
                printf("MEssage sent invalid \n");
                break;
        }
    }

    return true;
}


bool add_new_customer(int connFD) {
    struct Customer newCustomer;
    char buffer[1000];
    char wri[1024];
    ssize_t writeBytes;

    // Get the next available customer ID
    newCustomer.id = get_next_customer_id();

    // Prompt for customer details
    write(connFD, "&Enter Customer Name: \n", strlen("&Enter Customer Name: \n"));
    printf("Sent \n");
    memset(buffer,'\0',sizeof(buffer));
    recv(connFD, buffer, 1024,0);
    strcpy(newCustomer.name,buffer);
    printf("RE \n");
    newCustomer.name[strcspn(newCustomer.name, "\n")] = 0; // Remove newline

    write(connFD, "&Enter Customer Gender (M/F/O): \n", strlen("&Enter Customer Gender (M/F/O): \n"));
    printf("gender \n");
    char x='\0';
    read(connFD, &x, sizeof(x));
    newCustomer.gender=x;
    printf("RE \n");

    write(connFD, "&Enter Customer Age: \n", strlen("&Enter Customer Age: \n"));
    printf("age \n");
    memset(buffer,'\0',sizeof(buffer));
    recv(connFD, buffer,1024,0);
    printf("RE \n");
    newCustomer.age = atoi(buffer);

    write(connFD, "&Enter Customer Account Number: \n", strlen("&Enter Customer Account Number: \n"));
    printf("account \n");
    memset(buffer,'\0',sizeof(buffer));
    recv(connFD, buffer,1024,0);
    printf("RE \n");
    newCustomer.account = atoi(buffer);

    // Default password setup
    snprintf(newCustomer.password, sizeof(newCustomer.password), "%s-%d", newCustomer.name, newCustomer.id);
    newCustomer.active = 1;

    // Write new customer to file
    FILE *customerFile = fopen("custom.txt", "a+");
if (customerFile == NULL) {
    perror("Error opening customer file for writing!");
    return false;
}

// Write customer data in CSV format to the .txt file
    fprintf(customerFile, "%d,%s,%c,%d,%s,%d,%d\n", 
        newCustomer.id, 
        newCustomer.name, 
        newCustomer.gender, 
        newCustomer.age, 
        newCustomer.password,
        newCustomer.account, 
        newCustomer.active);
    send(connFD,"*SUCCESSFULLY ADDED\n",sizeof("*SUCCESSFULLY ADDED\n"),0);
    

fclose(customerFile);
    return true;
}



bool modify_customer_details(int connFD) {
    int customerID;
    struct Customer updatedCustomer;
    char buffer[2000];
    char line[256];

    // Prompt for Customer ID to modify
    send(connFD, "&Enter Customer ID to modify: \n", strlen("&Enter Customer ID to modify: \n"), 0);
    memset(buffer, '\0', sizeof(buffer));
    recv(connFD, buffer, sizeof(buffer), 0);
    customerID = atoi(buffer);

    // Open the customer file for reading
    FILE *file = fopen("custom.txt", "r");
    if (file == NULL) {
        perror("Error opening customer file for reading!");
        return false;
    }

    // Temporary file to hold updated customer details
    FILE *tempFile = fopen("temp_custom.txt", "w");
    if (tempFile == NULL) {
        perror("Error opening temporary file for writing!");
        fclose(file);
        return false;
    }

    // Flag to check if customer is found
    int found = 0;

    // Read through the file to find the customer record
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%d,%[^,],%c,%d,%[^,],%d,%d",
                   &updatedCustomer.id, updatedCustomer.name,
                   &updatedCustomer.gender, &updatedCustomer.age,
                   updatedCustomer.password, &updatedCustomer.account,
                   &updatedCustomer.active) == 7) {
            // Check if this is the customer to modify
            if (updatedCustomer.id == customerID) {
                found = 1; // Customer found
                break; // Exit loop to modify the details
            }
        }
    }

    fclose(file); // Close the file after reading

    // If the customer was not found
    if (!found) {
        send(connFD, "*Customer not found.\n", strlen("*Customer not found.\n"), 0);
        return false;
    }

    // Prepare to modify customer details
    int choice;

    // Prompt to modify customer details
    fflush(stdout);
    send(connFD, "&Select detail to modify:\n1. Name\n2. Gender\n3. Age\n0. Exit\n",
         strlen("&Select detail to modify:\n1. Name\n2. Gender\n3. Age\n0. Exit\n"), 0);

    // Receive choice from the client
    fflush(stdout);
    memset(buffer, '\0', sizeof(buffer));
    recv(connFD, buffer,sizeof(buffer), 0);
    printf("I read choice %s",buffer);
    choice = atoi(buffer);

    while (choice != 0) {
        switch (choice) {
            case 1: // Modify Name
                send(connFD, "&Enter new Customer Name (leave blank for no change):\n",
                     strlen("&Enter new Customer Name (leave blank for no change):\n"), 0);
                memset(buffer, '\0', sizeof(buffer));
                recv(connFD, buffer, sizeof(buffer), 0);
                if (strlen(buffer) > 0) {
                    strcpy(updatedCustomer.name, buffer);
                }
                break;
            case 2: // Modify Gender
                send(connFD, "&Enter new Customer Gender (M/F/O, leave blank for no change):\n",
                     strlen("&Enter new Customer Gender (M/F/O, leave blank for no change):\n"), 0);
                memset(buffer, '\0', sizeof(buffer));
                recv(connFD, buffer, sizeof(buffer), 0);
                if (strlen(buffer) > 0) {
                    updatedCustomer.gender = buffer[0];
                }
                break;
            case 3: // Modify Age
                send(connFD, "&Enter new Customer Age (leave blank for no change):\n",
                     strlen("&Enter new Customer Age (leave blank for no change):\n"), 0);
                memset(buffer, '\0', sizeof(buffer));
                recv(connFD, buffer, sizeof(buffer), 0);
                if (strlen(buffer) > 0) {
                    updatedCustomer.age = atoi(buffer);
                }
                break;
            default:
                send(connFD, "*Invalid choice. Please select a valid option.\n",
                     strlen("*Invalid choice. Please select a valid option.\n"), 0);
                break;
        }

        // Ask for another modification
        send(connFD, "&Select another detail to modify or 0 to finish:\n",
             strlen("&Select another detail to modify or 0 to finish:\n"), 0);
        memset(buffer, '\0', sizeof(buffer));
        recv(connFD, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);
    }

    // Reopen the customer file to update with modified details
    file = fopen("custom.txt", "r");
    if (file == NULL) {
        perror("Error opening customer file for reading!");
        fclose(tempFile);
        return false;
    }

    // Write modified customer details to the temporary file
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%d,%[^,],%c,%d,%[^,],%d,%d",
                   &updatedCustomer.id, updatedCustomer.name,
                   &updatedCustomer.gender, &updatedCustomer.age,
                   updatedCustomer.password, &updatedCustomer.account,
                   &updatedCustomer.active) == 7) {
            if (updatedCustomer.id == customerID) {
                // Write updated customer details
                fprintf(tempFile, "%d,%s,%c,%d,%s,%d,%d\n",
                        updatedCustomer.id, updatedCustomer.name,
                        updatedCustomer.gender, updatedCustomer.age,
                        updatedCustomer.password, updatedCustomer.account,
                        updatedCustomer.active);
            } else {
                // Write original line if it’s not modified
                fprintf(tempFile, "%s", line);
            }
        }
    }

    fclose(file);    // Close the original file
    fclose(tempFile); // Close the temporary file

    // Replace the original file with the updated temporary file
    rename("temp_custom.txt", "custom.txt");

    send(connFD, MODIFY_CUSTOMER_SUCCESS, strlen(MODIFY_CUSTOMER_SUCCESS), 0);
    return true;
}
bool change_p(int connFD) {
    FILE *file, *tempFile;
    struct Employee emp;
    int employeeID;
    char buffer[300];
    char oldPassword[50], newPassword[50], confirmPassword[50];
    bool found = false;

    // Open the employee file for reading
    file = fopen("employee.txt", "r");
    if (!file) {
        perror("Error opening employee file");
        return false;
    }

    // Create a temporary file to store updated employee data
    tempFile = fopen("temp.txt", "w");
    if (!tempFile) {
        perror("Error creating temporary file");
        fclose(file);
        return false;
    }
    send(connFD, "&Enter Employee ID to modify: \n", strlen("&Enter Employee ID to modify: \n"), 0);
    memset(buffer, '\0', sizeof(buffer));
    recv(connFD, buffer, sizeof(buffer), 0);
    employeeID = atoi(buffer);

    // Prompt for old password
    write(connFD, "&Enter your old password:\n", sizeof("&Enter your old password:\n"));
    read(connFD, oldPassword, sizeof(oldPassword));

    // Prompt for new password
    write(connFD, "&Enter your new password:\n",sizeof("&Enter your new password:\n"));
    read(connFD, newPassword, sizeof(newPassword));

    // Prompt to confirm new password
    write(connFD, "&Confirm your new password:\n",sizeof("&Confirm your new password:\n"));
    read(connFD, confirmPassword, sizeof(confirmPassword));

    // Read the employee file and search for the employee
    while (fgets(emp.name, sizeof(emp.name), file)) {
        char *token = strtok(emp.name, ","); // Read the first field (name)
        strcpy(emp.name, token);              // Store name
        token = strtok(NULL, ",");            // Read the second field (id)
        emp.id = atoi(token);                 // Convert to integer
        token = strtok(NULL, ",");            // Read the third field (type)
        emp.type = atoi(token);               // Convert to integer
        token = strtok(NULL, ",");            // Read the fourth field (password)
        strcpy(emp.password, token);          // Store password

        // Check if the employee ID matches
        if (emp.id == employeeID) {
            found = true; // Employee found
            // Validate old password
            if (strcmp(oldPassword, emp.password) == 0) {
                // Old password is correct; check new password
                if (strcmp(newPassword, confirmPassword) == 0) {
                    // Update the password
                    strcpy(emp.password, newPassword);
                    write(connFD, "Password changed successfully.\n", 32);
                } else {
                    write(connFD, "*New passwords do not match. Password not changed.\n", 51);
                }
            } else {
                write(connFD, "*Old password is incorrect. Password not changed.\n", 51);
            }
        }
        // Write the employee details back to the temporary file
        fprintf(tempFile, "%s,%d,%d,%s\n", emp.name, emp.id, emp.type, emp.password);
    }

    // Clean up
    fclose(file);
    fclose(tempFile);

    // Check if employee was found
    if (!found) {
        write(connFD, "*Employee not found.\n", 22);
        remove("temp.txt"); // Remove temporary file if not found
        return false;
    }

    // Replace the original employee file with the updated data
    remove("employee.txt"); // Delete the old employee file
    rename("temp.txt","employee.txt"); // Rename temp file to original file name

    return true;
}
void view_assigned_loan_applications(int connFD) {
    FILE *file = fopen("assloan.txt", "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    struct AssignedLoan loan;
    char buffer[BUFFER_SIZE];
    int employeeID;

    // Request employee ID from the client
    strcpy(buffer, "&Enter employee ID to view assigned loans:\n");
    send(connFD, buffer, strlen(buffer), 0);

    // Receive the employee ID from the client
    memset(buffer,'\0',sizeof(buffer));
    recv(connFD, buffer, BUFFER_SIZE, 0);
    employeeID = atoi(buffer);  // Convert received string to integer

    //printf("Received employee ID from client: %d\n", employeeID);
    
    // Prepare the message to send to the client
    memset(buffer,'\0',sizeof(buffer));
    snprintf(buffer, BUFFER_SIZE, "*Loans assigned to employee ID %d:\n", employeeID);
    send(connFD, buffer, strlen(buffer), 0);

    // Read and process each loan entry in the file
    while (fgets(buffer, BUFFER_SIZE, file)) {
        // Parse the line based on the format in assloan.txt
        sscanf(buffer, "%d,%d,%ld,%d,%d,%49[^,],%d",
               &loan.loanID, &loan.accountNumber, &loan.loanAmount,
               &loan.customerID, &loan.employeeID, loan.loanPurpose, &loan.status);

        // Check if the loan is assigned to the specified employee ID
        if (loan.employeeID == employeeID) {
            // Format loan details to send to the client
            snprintf(buffer, BUFFER_SIZE,
                     "*Loan ID: %d\nAccount Number: %d\nLoan Amount: %ld\nCustomer ID: %d\nPurpose: %s\nStatus: %d\n\n",
                     loan.loanID, loan.accountNumber, loan.loanAmount, loan.customerID, loan.loanPurpose, loan.status);
            send(connFD, buffer, strlen(buffer), 0);
        }
    }

    // Notify client of end of data transmission
    strcpy(buffer, "*End of loan applications.\n");
    send(connFD, buffer, strlen(buffer), 0);

    fclose(file);
}
void approve_reject_loan(int connFD) {
    FILE *file = fopen("assloan.txt", "r+");  // Open file for both reading and updating
    if (!file) {
        perror("Error opening file");
        return;
    }

    int employeeID;
    char buffer[BUFFER_SIZE];

    // Request the employee ID from the client
    strcpy(buffer, "&Enter your Employee ID:\n");
    send(connFD, buffer, strlen(buffer), 0);

    // Receive the employee ID from the client
    memset(buffer,'\0',sizeof(buffer));
    recv(connFD, buffer, BUFFER_SIZE, 0);
    employeeID = atoi(buffer);

    // Prepare to find and display loans for the given employeeID
    struct AssignedLoan loan;
    int found = 0;
    long int pos;

    while (fgets(buffer, BUFFER_SIZE, file)) {
        // Store the current file position for later use
        pos = ftell(file) - strlen(buffer);

        // Parse loan details from the file
        sscanf(buffer, "%d,%d,%ld,%d,%d,%49[^,],%d",
               &loan.loanID, &loan.accountNumber, &loan.loanAmount,
               &loan.customerID, &loan.employeeID, loan.loanPurpose, &loan.status);

        // Check if the loan is assigned to the employee and is pending (status == -1)
        if (loan.employeeID == employeeID && loan.status == -1) {
            found = 1;
            // Send loan details to the client
            snprintf(buffer, BUFFER_SIZE,
                     "Loan ID: %d\nAccount Number: %d\nLoan Amount: %ld\nCustomer ID: %d\nPurpose: %s\nStatus: Pending\n",
                     loan.loanID, loan.accountNumber, loan.loanAmount, loan.customerID, loan.loanPurpose);
            send(connFD, buffer, strlen(buffer), 0);

            // Ask for approval or rejection from the client
            strcpy(buffer, "Do you want to approve (1) or reject (0) this loan? Enter 1 or 0:\n");
            send(connFD, buffer, strlen(buffer), 0);
            memset(buffer,'\0',sizeof(buffer));
            recv(connFD, buffer, BUFFER_SIZE, 0);

            // Update the loan status based on client's response
            int decision = atoi(buffer);
            loan.status = (decision == 1) ? 1 : 0;

            // Seek to the beginning of the current line to overwrite it
            fseek(file, pos, SEEK_SET);
            fprintf(file, "%d,%d,%ld,%d,%d,%s,%d\n", 
                    loan.loanID, loan.accountNumber, loan.loanAmount, 
                    loan.customerID, loan.employeeID, loan.loanPurpose, loan.status);

            // Flush to ensure data is written
            fflush(file);

            // Inform client of the updated status
            snprintf(buffer, BUFFER_SIZE, "Loan ID %d has been %s.\n", loan.loanID, (loan.status == 1) ? "approved" : "rejected");
            send(connFD, buffer, strlen(buffer), 0);
        }
    }

    if (!found) {
        memset(buffer,'\0',sizeof(buffer));
        strcpy(buffer, "*No pending loans found for your employee ID.\n");
        send(connFD, buffer, strlen(buffer), 0);
    }

    fclose(file);
}



int get_next_customer_id(int connFD) {
    int max_id = 0;
    struct Customer c;
    

    
    FILE *file = fopen("custom.txt", "r+"); // Open the file in read mode

    if (file == NULL) {
        perror("Unable to open file");
        return -1; // Return an error code if the file can't be opened
    }

    char line[256]; // Buffer to hold each line from the file

    // Read through the file to find the maximum ID using fgets and sscanf
    while (fgets(line, sizeof(line), file)) {
        // Parse the CSV line and populate the employee struct
        if (sscanf(line, "%d,%[^,],%c,%d,%[^,],%d,%d", &c.id,c.name,&c.gender, &c.age,c.password,&c.account,&c.active) == 7) {
            if (c.id > max_id) {
                max_id = c.id; // Update max_id if a larger ID is found
            }
        }
    }

    fclose(file);
    //printf("%d",max_id+1);
    return max_id + 1; 
}
 

 #endif
