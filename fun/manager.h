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
#include <time.h>

#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"
#include "../employee.h"


#include "../fun/constants.h"
#define BUFFER_SIZE 1024

int changeEmployeePassword(int connFD);
int checkManagerCredentials(const char* loginID, const char* password);
int readEmployeeData();
void managerLogin(int connFD);
bool activateAccount(int connFD);
bool deactivateAccount(int connFD);
void reviewCustomerFeedback(int connFD);
void assignLoanApplication(int connFD);
bool getEmployeeByID(int d, struct Employee *emp);
void displayEmployeeIDs(int connFD);
void displayLoanRequests(int connFD);
struct Employee employees[MAX_EMPLOYEES];

// Function to check manager credentials
int checkManagerCredentials(const char* loginID, const char* password) {
    FILE *file = fopen("employee.txt", "r"); // Open the employee data file in text format
    if (file == NULL) {
        printf("Error opening file.\n");
        return 0; // Return 0 if the file can't be opened
    }

    struct Employee emp;
    char line[256]; // Buffer to hold each line from the file

    // Read through the file to find the maximum ID using fgets and sscanf
    while (fgets(line, sizeof(line), file)) {
        // Parse the CSV line and populate the employee struct
        if (sscanf(line, "%[^,],%d,%d,%[^,]",  emp.name,&emp.id,&emp.type,emp.password) == 4 ){
            if (emp.id==atoi(loginID) && emp.type==1) {
                fclose(file);
                return 1; // Update max_id if a larger ID is found
            }
        }
    }

    fclose(file); // Close the file after reading
    return 0; // Return 0 if no matching credentials are found
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
    char password[1000];
    int loginAttempts = 0;  // Initialize login attempts counter
    int choice;
    char buffer[256];       // Buffer for reading and writing data

    // Welcome message
    // strcpy(buffer, "*");
    // strcat(buffer, MANAGER_LOGIN_WELCOME);
    // write(connFD, buffer, strlen(buffer));

    
        // Request login ID
        fflush(stdout);
        send(connFD,LOGIN_ID, strlen(LOGIN_ID),0);
        memset(loginID,'\0',sizeof(loginID));
        read(connFD, loginID, sizeof(loginID));
        printf("loginid %s \n",loginID);

        // Request password
        fflush(stdout);
        send(connFD,PASSWORD,sizeof(PASSWORD),0);
        // memset(password,'\0',sizeof(password));
        bzero(password, sizeof(password));
        read(connFD, password, sizeof(password));
        printf("Password %s \n",password);

        // Validate credentials
        if (checkManagerCredentials(loginID, password)) {
            
            //write(connFD,MANAGER_LOGIN_SUCCESS, strlen(MANAGER_LOGIN_SUCCESS));
            bool logged_in=true;

            while (logged_in) {
                // Send manager menu options to the client
                fflush(stdout);
                //strcat(buffer, MANAGER_MENU);
                write(connFD,MANAGER_MENU, strlen(MANAGER_MENU));

                memset(buffer,'\0',sizeof(buffer));
                read(connFD,buffer, sizeof(buffer));
                choice=atoi(buffer);

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
                        //changeEmployeePassword(connFD);
                        break;
                    case 5: // Deactivate Account
                        deactivateAccount(connFD);
                        break;
                    case 6: // Logout
                        //logout(connFD);
                        logged_in=false;
                        break; // Exit the manager session
                    default:
                        // strcpy(buffer, "*");
                        // strcat(buffer, MANAGER_LOGOUT)
                         write(connFD,"*Invalid choice\n", strlen("*Invalid choice\n"));
                        return;
                }
            }
        } else {
          
            
            strcat(buffer, INVALID_LOGIN);
            write(connFD, buffer, strlen(buffer));  // Inform about invalid login
        }
    }






// // Function to activate a customer account using system calls
bool activateAccount(int connFD) {
    FILE *file, *tempFile;
    struct Account acc;
    bool found = false;

    char buffer[BUFFER_SIZE];
    int accountNumber;

    // Prompt the customer for their account number
    write(connFD, "&Enter your account number to activate: \n", 41);
    bzero(buffer, sizeof(buffer)); // Clear the buffer

    // Read account number from the client
    ssize_t bytesRead = read(connFD, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        perror("Error reading account number from client");
        return false;
    }
    
    // Convert the input to an integer
    accountNumber = atoi(buffer);

    // Open the account file for reading
    file = fopen("acc.txt", "r");
    if (!file) {
        perror("Error opening account file");
        return false;
    }

    // Create a temporary file to store updated account data
    tempFile = fopen("temp.txt", "w");
    if (!tempFile) {
        perror("Error creating temporary file");
        fclose(file);
        return false;
    }

    // Read the account file and search for the account
    while (fscanf(file, "%d,%d,%d,%ld,%d\n", 
                  &acc.accountNumber, 
                  &acc.customerid, 
                  &acc.active,  // Read active as int
                  &acc.balance, 
                  &acc.transactionCount) == 5) {

        // Check if the account number matches
        if (acc.accountNumber == accountNumber) {
            found = true; // Account found
            acc.active = 1; // Activate the account (set active to 1)
            write(connFD, "*Account activated successfully.\n",sizeof("*Account activated successfully.\n"));
        }

        // Write the account details back to the temporary file
        fprintf(tempFile, "%d,%d,%d,%ld,%d\n", 
                acc.accountNumber, 
                acc.customerid, 
                acc.active,  // Write active as int
                acc.balance, 
                acc.transactionCount);
    }

    // Clean up
    fclose(file);
    fclose(tempFile);

    // Check if account was found
    if (!found) {
        write(connFD, "*Account not found.\n", 21);
        remove("temp.txt"); // Remove temporary file if not found
        return false;
    }

    // Replace the original account file with the updated data
    remove("acc.txt"); // Delete the old account file
    rename("temp.txt", "acc.txt"); // Rename temp file to original file name

    return true;
}
// Function to deactivate an account
bool deactivateAccount(int connFD) {
    FILE *file, *tempFile;
    struct Account acc;
    bool found = false;
    char buffer[BUFFER_SIZE];
    int accountNumber;

    // Prompt the customer for their account number
    write(connFD, "&Enter your account number to deactivate: \n", 43);
    bzero(buffer, sizeof(buffer)); // Clear the buffer

    // Read account number from the client
    ssize_t bytesRead = read(connFD, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        perror("Error reading account number from client");
        return false;
    }
    
    // Convert the input to an integer
    accountNumber = atoi(buffer);

    // Open the account file for reading
    file = fopen("acc.txt", "r");
    if (!file) {
        perror("Error opening account file");
        return false;
    }

    // Create a temporary file to store updated account data
    tempFile = fopen("temp.txt", "w");
    if (!tempFile) {
        perror("Error creating temporary file");
        fclose(file);
        return false;
    }

    // Read the account file and search for the account
    while (fscanf(file, "%d,%d,%d,%ld,%d\n", 
                  &acc.accountNumber, 
                  &acc.customerid, 
                  &acc.active,  // Read active as int
                  &acc.balance, 
                  &acc.transactionCount) == 5) {

        // Check if the account number matches
        if (acc.accountNumber == accountNumber) {
            found = true; // Account found
            acc.active = 0; // Deactivate the account (set active to 0)
            write(connFD, "*Account deactivated successfully.\n",sizeof("*Account deactivated successfully.\n"));
        }

        // Write the account details back to the temporary file
        fprintf(tempFile, "%d,%d,%d,%ld,%d\n", 
                acc.accountNumber, 
                acc.customerid, 
                acc.active,  // Write active as int
                acc.balance, 
                acc.transactionCount);
    }

    // Clean up
    fclose(file);
    fclose(tempFile);

    // Check if account was found
    if (!found) {
        write(connFD, "*Account not found.\n", 21);
        remove("temp.txt"); // Remove temporary file if not found
        return false;
    }

    // Replace the original account file with the updated data
    remove("acc.txt"); // Delete the old account file
    rename("temp.txt","acc.txt"); // Rename temp file to original file name

    return true;
}
void reviewCustomerFeedback(int connFD) {
    FILE *file;
    struct Feedback feedback;
    char buffer[BUFFER_SIZE];

    // Open the feedback file for reading
    file = fopen("feed.txt", "r");
    if (!file) {
        perror("Error opening feedback file");
        write(connFD, "*Error opening feedback file.\n", 31);
        return;
    }

    // Send a header to the client
    //write(connFD, "Customer Feedback:\n", 20);

    // Read feedback data from the file
    while (fscanf(file, "%d,%d,%d,%499[^,],%ld\n", 
                  &feedback.customerID, 
                  &feedback.feedbackID, 
                  &feedback.accountNumber, 
                  feedback.feedbackText, 
                  &feedback.feedbackDate) == 5) {

        // Prepare feedback details for sending to the client
        snprintf(buffer, sizeof(buffer),
                 "*Customer ID: %d | Feedback ID: %d | Account Number: %d\n"
                 "Feedback: %s\n"
                 "Date: %s\n\n",
                 feedback.customerID,
                 feedback.feedbackID,
                 feedback.accountNumber,
                 feedback.feedbackText,
                 ctime(&feedback.feedbackDate)); // Convert time_t to string

        // Send feedback details to the client
        write(connFD, buffer, strlen(buffer));
    }

    // Clean up
    fclose(file);
}
void displayLoanRequests(int connFD) {
    FILE *loanFile = fopen("rloan.txt", "r");
    if (!loanFile) {
        send(connFD, "*Error: Unable to open loan requests file.\n", 44, 0);
        return;
    }

    char line[1024];
    char response[1024];
    int responseLength = 0;

    // Prepare the response header
    responseLength += snprintf(response + responseLength, sizeof(response) - responseLength,
                                "*Loan IDs:\n");

    // Read and extract loan IDs
    while (fgets(line, sizeof(line), loanFile)) {
        struct Loan loan;
        // Parse the CSV line into the loan structure
        sscanf(line, "%d,%d,%ld,%d,%d,%[^\n]", &loan.loanID, &loan.accountNumber, &loan.loanAmount,
               &loan.customerID, &loan.isAssigned, loan.loanPurpose);
        
        // Append the loan ID to the response
        responseLength += snprintf(response + responseLength, sizeof(response) - responseLength,
                                    "%d\n", loan.loanID);
    }

    fclose(loanFile);

    // Send the loan IDs to the client
    send(connFD, response, responseLength, 0);
}
void displayEmployeeIDs(int connFD) {
    FILE *employeeFile = fopen("employee.txt", "r");
    if (!employeeFile) {
        const char *error_message = "*Error: Unable to open employee file.\n";
        send(connFD, error_message, strlen(error_message), 0);
        return;
    }

    char line[1024];
    char response[1024] = "*Employee IDs:\n"; // Initialize response with header
    int responseLength = strlen(response);
    int found = 0;

    // Read each line from the employee file
    while (fgets(line, sizeof(line), employeeFile)) {
        struct Employee employee;
        
        // Read employee details from the line
        sscanf(line, "%49[^,],%d,%d,%49[^\n]", employee.name, &employee.id, &employee.type, employee.password);
        
        // Append the employee ID to the response
        if(employee.type==0){
        responseLength += snprintf(response + responseLength, sizeof(response) - responseLength,
                                   "%d  \n", employee.id);
        found = 1; // Mark that at least one employee was found
    }
}

    fclose(employeeFile);

    // If no employee was found, inform the client
    if (!found) {
        const char *no_employees_message = "*No employees found.\n";
        send(connFD, no_employees_message, strlen(no_employees_message), 0);
    } else {
        // Send the response containing all employee IDs
        send(connFD, response, responseLength, 0);
    }
}
// Function to assign loan applications to an employee
void assignLoanApplication(int connFD) {
    // Display available employee IDs
    displayEmployeeIDs(connFD);

    // Display available loan requests
    displayLoanRequests(connFD);
    FILE *assignedFile = fopen("assloan.txt", "a");
    if (!assignedFile) {
        send(connFD, "Error: Unable to open loan assigned file.\n", 44, 0);
        return;
    }

    // Open loan request file for reading
    FILE *loanFile = fopen("rloan.txt", "r");
    if (!loanFile) {
        send(connFD, "Error: Unable to open loan requests file.\n", 44, 0);
        fclose(assignedFile);
        return;
    }


    // Prepare to assign loan to employee
    char line[1024];
    int loanID, employeeID;

    // Request employee ID to assign the loan
    fflush(stdout);
    send(connFD, "&Enter Employee ID to assign the loan: \n", 41, 0);
    memset(line,'\0',sizeof(line));
    read(connFD, line, sizeof(line));
    printf("I %s",line);
    employeeID = atoi(line);

    // Ask for the Loan ID to assign
    fflush(stdout);
    send(connFD, "&Enter Loan ID to assign: \n", 28, 0);
    memset(line,'\0',sizeof(line));
    read(connFD, line, sizeof(line));
    printf("I %s",line);
    loanID = atoi(line);

    // Open loan assigned file for writing
   
    // Create a temporary file for updating loan requests
    FILE *tempFile = fopen("temp_loan_requests.txt", "w");
    if (!tempFile) {
        send(connFD, "Error: Unable to create temporary file.\n", 41, 0);
        fclose(loanFile);
        fclose(assignedFile);
        return;
    }

    char loanLine[1024];
    int found = 0;

    // Process loans
    while (fgets(loanLine, sizeof(loanLine), loanFile)) {
        struct Loan loan;

        // Read loan details
        sscanf(loanLine, "%d,%d,%ld,%d,%d,%[^\n]", &loan.loanID, &loan.accountNumber, 
               &loan.loanAmount, &loan.customerID, &loan.isAssigned, loan.loanPurpose);
        
        if (loan.loanID == loanID) {
            // Loan matches the one to assign
            fprintf(assignedFile, "%d,%d,%ld,%d,%d,%s%d\n", loan.loanID, loan.accountNumber, 
                    loan.loanAmount, loan.customerID, employeeID, loan.loanPurpose,-1);
            found = 1; // Mark that we found and assigned this loan
        } else {
            // Keep the loan in requests
            fprintf(tempFile, "%d,%d,%ld,%d,%d,%s\n", loan.loanID, loan.accountNumber, 
                    loan.loanAmount, loan.customerID, loan.isAssigned, loan.loanPurpose);
        }
    }

    fclose(loanFile);
    fclose(tempFile);
    fclose(assignedFile);

    // Replace old request file with the updated one
    remove("rloan.txt");
    rename("temp_loan_requests.txt", "rloan.txt");

    // Inform the user
    if (found) {
        snprintf(line, sizeof(line), "*Loan ID %d assigned to Employee ID %d.\n", loanID, employeeID);
    } else {
        snprintf(line, sizeof(line), "*Loan ID %d not found in requests.\n", loanID);
    }
    send(connFD, line, strlen(line), 0);
}


// void reviewCustomerFeedback(int connFD) {
//     int feedbackFile;
//     struct Feedback feedback;
//     ssize_t bytesRead;
//     int feedbackCount = 0;
//     char buffer[1024];  // Buffer to send data to the client

//     // Open the feedback file for reading
//     feedbackFile = open(FEEDBACK_FILE, O_RDONLY);
//     if (feedbackFile == -1) {
//         strcpy(buffer, "*Error opening feedback file.\n");
//         write(connFD, buffer, strlen(buffer));
//         return;
//     }

//     // Send the feedback review prompt to the client
//     strcpy(buffer, MANAGER_REVIEW_FEEDBACK_PROMPT);
//     write(connFD, buffer, strlen(buffer));

//     // Read each feedback from the file
//     while ((bytesRead = read(feedbackFile, &feedback, sizeof(struct Feedback))) > 0) {
//         feedbackCount++;

//         // Prepare and send feedback details to the client
//         sprintf(buffer, "*Feedback ID: %d\n", feedback.feedbackID);
//         write(connFD, buffer, strlen(buffer));

//         sprintf(buffer, "*Account Number: %d\n", feedback.accountNumber);
//         write(connFD, buffer, strlen(buffer));

//         sprintf(buffer, "*Feedback: %s\n", feedback.feedbackText);
//         write(connFD, buffer, strlen(buffer));

//         // Convert feedback date to a readable format and send it
//         sprintf(buffer, "*Date: %s", ctime(&feedback.feedbackDate));  // ctime() adds newline automatically
//         write(connFD, buffer, strlen(buffer));

//         // Send separator line
//         strcpy(buffer, "*--------------------------------------------------\n");
//         write(connFD, buffer, strlen(buffer));
//     }

//     // Check if there was an error while reading the file
//     if (bytesRead == -1) {
//         strcpy(buffer, "*Error reading feedback file.\n");
//         write(connFD, buffer, strlen(buffer));
//     } else if (feedbackCount == 0) {
//         // If no feedback records were found, notify the client
//         strcpy(buffer, NO_FEEDBACKS_AVAILABLE);
//         write(connFD, buffer, strlen(buffer));
//     }

//     // Close the feedback file
//     close(feedbackFile);
// }


  // End after incorrect old password


#endif






