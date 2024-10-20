#ifndef MANAGER_H
#define MANAGER_H

#include <stdio.h>
#include <fcntl.h>    // For open()
#include <unistd.h>   // For close(), read(), write()
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"
#include "../employee.h"


#include "../fun/constants.h"

int changeEmployeePassword(int connFD, const char* loginID);
int checkManagerCredentials(const char* loginID, const char* password);
int readEmployeeData();
void managerLogin(int connFD);
void activateAccount(int connFD);
void deactivateAccount(int connFD);
void reviewCustomerFeedback(int connFD);
void assignLoanApplication(int connFD);
bool getEmployeeByID(int d, struct Employee *emp);
struct Employee employees[MAX_EMPLOYEES];

// Function to check manager credentials
int checkManagerCredentials(const char* loginID, const char* password) {
    
    int employeeCount = readEmployeeData();

    // Loop through all employee records to find the matching loginID and password
    for (int i = 0; i < employeeCount; ++i) {
        // Convert employee ID to string for comparison with loginID
        char idStr[10];
        snprintf(idStr, sizeof(idStr), "%d", employees[i].id);

        // Check if loginID and password match
        if (strcmp(idStr, loginID) == 0 && strcmp(employees[i].password, password) == 0) {
            // Check if the employee is a manager
            if (employees[i].type == 1) {
                return 1;  // Manager login successful
            } else {
                return 0;  // Not a manager
            }
        }
    }

    // If no match is found, return 0 (login failure)
    return 0;
}

// Function to read employee data from file

int readEmployeeData() {
    int fileFD = open(EMPLOYEE_FILE, O_RDONLY);
    if (fileFD < 0) {
        perror("Unable to open employee file");
        return 0;
    }

    int count = 0;
    ssize_t bytesRead;
    while ((bytesRead = read(fileFD, &employees[count], sizeof(struct Employee))) > 0) {
        if (bytesRead != sizeof(struct Employee)) {
            perror("Partial read error");
            break;
        }
        count++;
        if (count >= MAX_EMPLOYEES) {
            break;  // Avoid reading beyond array size
        }
    }

    if (bytesRead < 0) {
        perror("Error reading employee file");
    }

    close(fileFD);
    return count;
}



// Function for manager login

void managerLogin(int connFD) {
    char loginID[100];
    char password[100];
    int loginAttempts = 0;  // Initialize login attempts counter
    int choice;
    char buffer[256];       // Buffer for reading and writing data

    // Welcome message
    strcpy(buffer, "*");
    strcat(buffer, MANAGER_LOGIN_WELCOME);
    write(connFD, buffer, strlen(buffer));

    while (loginAttempts < MAX_LOGIN_ATTEMPTS) {
        // Request login ID
        strcpy(buffer, "&");
        strcat(buffer, LOGIN_ID);
        write(connFD, buffer, strlen(buffer));
        read(connFD, loginID, sizeof(loginID));

        // Request password
        strcpy(buffer, "&");
        strcat(buffer, PASSWORD);
        write(connFD, buffer, strlen(buffer));
        read(connFD, password, sizeof(password));

        // Validate credentials
        if (checkManagerCredentials(loginID, password)) {
            strcpy(buffer, "*");
            strcat(buffer, MANAGER_LOGIN_SUCCESS);
            write(connFD, buffer, strlen(buffer));

            while (1) {
                // Send manager menu options to the client
                strcpy(buffer, "*");
                strcat(buffer, MANAGER_MENU);
                write(connFD, buffer, strlen(buffer));

                // Read choice from client
                strcpy(buffer, "&");
                strcat(buffer, "Enter your choice: ");
                write(connFD, buffer, strlen(buffer));
                read(connFD, &choice, sizeof(int));

                switch (choice) {
                    case 1: // Activate/Deactivate Customer Account
                        activateAccount(connFD);  // Pass connFD to handle socket communication
                        break;
                    case 2: // Assign Loan Applications
                        assignLoanApplication(connFD);
                        break;
                    case 3: // Review Customer Feedback
                        reviewCustomerFeedback(connFD);  // Call feedback review function
                        break;
                    case 4: // Change Password
                        changeEmployeePassword(connFD, loginID);
                        break;
                    case 5: // Deactivate Account
                        deactivateAccount(connFD);
                        break;
                    case 6: // Logout
                        //logout(connFD);
                        break; // Exit the manager session
                    default:
                        strcpy(buffer, "*");
                        strcat(buffer, MANAGER_LOGOUT);
                        write(connFD, buffer, strlen(buffer));
                        return;
                }
            }
        } else {
            loginAttempts++;  // Increment login attempts
            if (loginAttempts >= MAX_LOGIN_ATTEMPTS) {
                strcpy(buffer, "*");
                strcat(buffer, LOGIN_ATTEMPT_EXCEEDED);
                write(connFD, buffer, strlen(buffer));
                return;  // Exit if attempts exceeded
            }
            strcpy(buffer, "*");
            strcat(buffer, INVALID_LOGIN);
            write(connFD, buffer, strlen(buffer));  // Inform about invalid login
        }
    }
}





// Function to activate a customer account using system calls
void activateAccount(int connFD) {
    int accountID;
    struct Account account;
    FILE *file;
    char buffer[1024];
    bool accountFound = false;

    // Ask for the account ID to activate
    strcpy(buffer, "&");
    strcat(buffer, "Enter the account ID to activate: ");
    write(connFD, buffer, strlen(buffer));

    // Read account ID from the client
    read(connFD, &accountID, sizeof(int));

    // Open the account database file in read/write mode
    file = fopen("ACCOUNT_FILE", "rb+");  // Assuming accounts are stored in 'accounts.dat'
    if (file == NULL) {
        strcpy(buffer, "*");
        strcat(buffer, "Error: Could not open accounts database.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Search for the account in the file
    while (fread(&account, sizeof(struct Account), 1, file)) {
        if (account.accountNumber == accountID) {
            accountFound = true;
            break;
        }
    }

    if (!accountFound) {
        strcpy(buffer, "*");
        strcat(buffer, "Error: Account not found.\n");
        write(connFD, buffer, strlen(buffer));
        fclose(file);
        return;
    }

    // Check if the account is already active
    if (account.active) {
        strcpy(buffer, "*");
        strcat(buffer, "Account is already active.\n");
        write(connFD, buffer, strlen(buffer));
        fclose(file);
        return;
    }

    // Activate the account
    account.active = true;

    // Move the file pointer to the location of the account record and update it
    fseek(file, -sizeof(struct Account), SEEK_CUR);
    fwrite(&account, sizeof(struct Account), 1, file);

    // Close the file
    fclose(file);

    // Send success message to client
    strcpy(buffer, "*");
    strcat(buffer, "Account activated successfully.\n");
    write(connFD, buffer, strlen(buffer));
}
void deactivateAccount(int connFD) {
    int accountID;
    struct Account account;
    FILE *file;
    char buffer[1024];
    bool accountFound = false;

    // Ask for the account ID to deactivate
    strcpy(buffer, "&");
    strcat(buffer, "Enter the account ID to deactivate: ");
    write(connFD, buffer, strlen(buffer));

    // Read account ID from the client
    read(connFD, &accountID, sizeof(int));

    // Open the account database file in read/write mode
    file = fopen("ACCOUNT_FILE", "rb+");  // Assuming accounts are stored in 'accounts.dat'
    if (file == NULL) {
        strcpy(buffer, "*");
        strcat(buffer, "Error: Could not open accounts database.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Search for the account in the file
    while (fread(&account, sizeof(struct Account), 1, file)) {
        if (account.accountNumber == accountID) {
            accountFound = true;
            break;
        }
    }

    if (!accountFound) {
        strcpy(buffer, "*");
        strcat(buffer, "Error: Account not found.\n");
        write(connFD, buffer, strlen(buffer));
        fclose(file);
        return;
    }

    // Check if the account is already deactivated
    if (!account.active) {
        strcpy(buffer, "*");
        strcat(buffer, "Account is already deactivated.\n");
        write(connFD, buffer, strlen(buffer));
        fclose(file);
        return;
    }

    // Deactivate the account
    account.active = false;

    // Move the file pointer to the location of the account record and update it
    fseek(file, -sizeof(struct Account), SEEK_CUR);
    fwrite(&account, sizeof(struct Account), 1, file);

    // Close the file
    fclose(file);

    // Send success message to client
    strcpy(buffer, "*");
    strcat(buffer, "Account deactivated successfully.\n");
    write(connFD, buffer, strlen(buffer));
}

void assignLoanApplication(int connFD) {
    int loanFile;
    int tempFile;
    struct Loan loanApp;
    int employeeID;
    int foundUnassigned = 0;
    ssize_t bytesRead;
    char buffer[1024];
    struct Employee employee;
    int loanAssigned = 0;  // To track if a loan has been assigned

    // Ask for employee ID
    strcpy(buffer, "&Enter the employee ID: ");  // Prompt for employee ID
    write(connFD, buffer, strlen(buffer));

    // Clear buffer before reading employee ID
    bzero(buffer, sizeof(buffer));
    read(connFD, buffer, sizeof(buffer));  // Read employee ID from client
    employeeID = atoi(buffer);  // Convert input to integer

    // Open the employee file and fetch the employee details

    if (!getEmployeeByID(employeeID, &employee)) {
        // Send error message if employee not found
        strcpy(buffer, "*Employee not found.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Check if the employee already has 10 loans assigned
    int assignedLoanCount = 0;
    for (int i = 0; i < LOAN_PURPOSE_MAX_LEN ; i++) {
        if (employee.l[i] != -1) {
            assignedLoanCount++;
        }
    }

    if (assignedLoanCount >= MAX_LOANS_PER_EMPLOYEE) {
        // Send error message if employee has already been assigned 5 loans
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*This employee already has the maximum number of loans assigned (5).\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Open the loan application file for reading
    loanFile = open(LOAN_FILE, O_RDONLY);
    if (loanFile == -1) {
        // Send error message to client
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Error opening loan application file.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Open a temporary file for writing
    tempFile = open("temp_loan.bank", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tempFile == -1) {
        // Send error message to client
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Error opening temporary file.\n");
        write(connFD, buffer, strlen(buffer));
        close(loanFile);
        return;
    }

    // Read through the loan application file and list unassigned loans
     bzero(buffer, sizeof(buffer));
    strcpy(buffer, "*The following loans are not assigned yet:\n");
    write(connFD, buffer, strlen(buffer));

    while ((bytesRead = read(loanFile, &loanApp, sizeof(struct Loan))) > 0) {
        // Clear loanApp buffer before next read
        bzero(&loanApp, sizeof(struct Loan));

        if (loanApp.isAssigned == 0) {
            // List unassigned loan applications
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "*Loan ID: %d\n", loanApp.loanID);
            write(connFD, buffer, strlen(buffer));
            foundUnassigned = 1;
        }
    }

    // If no unassigned loans were found
    if (!foundUnassigned) {
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*No unassigned loan applications available.\n");
        write(connFD, buffer, strlen(buffer));
        close(loanFile);
        close(tempFile);
        return;
    }

    // Now ask the client to enter a loan ID to assign
     bzero(buffer, sizeof(buffer));
    strcpy(buffer, "&Enter the loan ID you want to assign: ");
    write(connFD, buffer, strlen(buffer));

    // Read the loan ID to be assigned from the client
    bzero(buffer, sizeof(buffer));
    read(connFD, buffer, sizeof(buffer));
    int loanIDToAssign = atoi(buffer);  // Convert input to integer

    // Rewind the file pointer and search for the selected loan ID
    lseek(loanFile, 0, SEEK_SET);
    while ((bytesRead = read(loanFile, &loanApp, sizeof(struct Loan))) > 0) {
        if (loanApp.loanID == loanIDToAssign && loanApp.isAssigned == 0) {
            // Assign the loan to the employee
            loanApp.isAssigned = 1;

            //loanApp.assignedEmployeeID = employeeID;

            // Add the loan ID to the employee's assigned loans
            for (int i = 0; i < MAX_LOANS_PER_EMPLOYEE; i++) {
                if (employee.l[i] == -1) {
                    employee.l[i] = loanApp.loanID;
                    loanAssigned = 1;
                    break;
                }
            }

            // Write the updated loan application to the temp file
            if (write(tempFile, &loanApp, sizeof(struct Loan)) == -1) {
                strcpy(buffer, "*Error writing to temporary file.\n");
                write(connFD, buffer, strlen(buffer));
                close(loanFile);
                close(tempFile);
                return;
            }

            break;  // Stop searching after the loan has been assigned
        }
    }

    if (!loanAssigned) {
        // Send error message if loan couldn't be assigned
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Loan assignment failed. Make sure the loan ID is valid.\n");
        write(connFD, buffer, strlen(buffer));
    } else {
        // Send success message to the client
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Loan application assigned successfully.\n");
        write(connFD, buffer, strlen(buffer));
    }

    // Close the files
    close(loanFile);
    close(tempFile);

    // Replace the old file with the new one
    if (unlink(LOAN_FILE) == -1) {
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Error deleting original loan application file.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }
    if (rename("temp_loan.bank", LOAN_FILE) == -1) {
         bzero(buffer, sizeof(buffer));
        strcpy(buffer, "*Error renaming temporary file to loan application file.\n");
        write(connFD, buffer, strlen(buffer));
    }
}

void reviewCustomerFeedback(int connFD) {
    int feedbackFile;
    struct Feedback feedback;
    ssize_t bytesRead;
    int feedbackCount = 0;
    char buffer[1024];  // Buffer to send data to the client

    // Open the feedback file for reading
    feedbackFile = open(FEEDBACK_FILE, O_RDONLY);
    if (feedbackFile == -1) {
        strcpy(buffer, "*Error opening feedback file.\n");
        write(connFD, buffer, strlen(buffer));
        return;
    }

    // Send the feedback review prompt to the client
    strcpy(buffer, MANAGER_REVIEW_FEEDBACK_PROMPT);
    write(connFD, buffer, strlen(buffer));

    // Read each feedback from the file
    while ((bytesRead = read(feedbackFile, &feedback, sizeof(struct Feedback))) > 0) {
        feedbackCount++;

        // Prepare and send feedback details to the client
        sprintf(buffer, "*Feedback ID: %d\n", feedback.feedbackID);
        write(connFD, buffer, strlen(buffer));

        sprintf(buffer, "*Account Number: %d\n", feedback.accountNumber);
        write(connFD, buffer, strlen(buffer));

        sprintf(buffer, "*Feedback: %s\n", feedback.feedbackText);
        write(connFD, buffer, strlen(buffer));

        // Convert feedback date to a readable format and send it
        sprintf(buffer, "*Date: %s", ctime(&feedback.feedbackDate));  // ctime() adds newline automatically
        write(connFD, buffer, strlen(buffer));

        // Send separator line
        strcpy(buffer, "*--------------------------------------------------\n");
        write(connFD, buffer, strlen(buffer));
    }

    // Check if there was an error while reading the file
    if (bytesRead == -1) {
        strcpy(buffer, "*Error reading feedback file.\n");
        write(connFD, buffer, strlen(buffer));
    } else if (feedbackCount == 0) {
        // If no feedback records were found, notify the client
        strcpy(buffer, NO_FEEDBACKS_AVAILABLE);
        write(connFD, buffer, strlen(buffer));
    }

    // Close the feedback file
    close(feedbackFile);
}


int changeEmployeePassword(int connFD, const char* loginID) {
    char oldPassword[100];
    char newPassword[100];
    char confirmPassword[100];
    char buffer[1024];     // Buffer for sending and receiving data
     char str[20];  

    // Ask for old password
    strcpy(buffer, "&Enter your old password: ");  // Prompt for old password
    write(connFD, buffer, strlen(buffer));

    // Clear the buffer before reading old password
    bzero(oldPassword, sizeof(oldPassword));
    read(connFD, oldPassword, sizeof(oldPassword));  // Read old password from client

    // Check if the old password is correct
    for (int i = 0; i<readEmployeeData(); i++) {
          snprintf(str, sizeof(str), "%d", employees->l[i]); 
        if (strcmp(str, loginID) == 0 && strcmp(employees[i].password, oldPassword) == 0) {
            // Ask for new password
            strcpy(buffer, "&Enter your new password: ");  // Prompt for new password
            write(connFD, buffer, strlen(buffer));

            // Clear the buffer before reading new password
            bzero(newPassword, sizeof(newPassword));
            read(connFD, newPassword, sizeof(newPassword));  // Read new password from client

            // Ask for confirmation of new password
            strcpy(buffer, "&Re-enter your new password: ");  // Prompt for confirming new password
            write(connFD, buffer, strlen(buffer));

            // Clear the buffer before reading confirmation password
            bzero(confirmPassword, sizeof(confirmPassword));
            read(connFD, confirmPassword, sizeof(confirmPassword));  // Read confirmation password from client

            // Check if new password matches confirmation
            if (strcmp(newPassword, confirmPassword) == 0) {
                // Change the password
                strcpy(employees[i].password, newPassword);

                // Inform the client that the password change was successful
                strcpy(buffer, "*Password changed successfully.\n");
                write(connFD, buffer, strlen(buffer));
            } else {
                // Inform the client that the passwords do not match
                strcpy(buffer, "*The new passwords do not match. Please try again.\n");
                write(connFD, buffer, strlen(buffer));
            }
            return 0;  // End after processing password change
        }
    }

    // If old password is incorrect, inform the client
    strcpy(buffer, "*Old password is incorrect.\n");
    write(connFD, buffer, strlen(buffer));

    return 0;  // End after incorrect old password
}
bool getEmployeeByID(int d, struct Employee *emp){
    int f=open(EMPLOYEE_FILE,O_RDONLY);
    ssize_t bytesRead;
    struct Employee temp;
    while ((bytesRead = read(f, &temp, sizeof(struct Employee))) > 0){
        if(emp->id==d) {
            *emp = temp;
            return true;
        }
    }
    return false;
}
#endif






