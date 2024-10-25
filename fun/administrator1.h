#ifndef ADMIN_H
#define ADMIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


#include "../account.h"
#include "../customer.h"
#include "../transaction.h"
#include "../loan.h"
#include "../feedback.h"
#include "../employee.h"
#include "/home/tushar/Banking/admin.h"


#include "../fun/constants.h"


void adminLogin(int connFD);
void addNewEmployee(int connFD);
//void getAccountDetails(int connFD);
void manageUserRoles(int connFD);
void modifyEmployeeDetails(int connFD);
//void addAccount(int connFD);
//void deleteAccount(int connFD);
void modifyCustomerInfo(int connFD);
void changePassword(int connFD);
void adminLogout(int connFD);
void getEmployeeDetails(int connFD);
int generate_id();


void adminLogin(int connFD) {
    char loginID[100];
    char password[100];
    char server_message[2000], client_message[2000];

    // Get login ID from client
    fflush(stdout);
    fflush(stdin);
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    strcpy(server_message, " &Enter the LOGIN-ID for admin \n");
    send(connFD, server_message,strlen(server_message),0);

    recv(connFD,client_message,sizeof(client_message),0);
    printf("\n Username received : %s \n", client_message);
    strcpy(loginID,client_message);

    // Get password from client
    fflush(stdout);
    fflush(stdin);
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    strcpy(server_message, "&Enter the PASSWORD \n");
    send(connFD, server_message,strlen(server_message),0);

    recv(connFD,client_message,sizeof(client_message),0);
    printf("\n Password received : %s \n", client_message);
    strcpy(password,client_message);
    fflush(stdout);
    fflush(stdin);


    if (strcmp(loginID, ADMIN_LOGIN_ID) == 0 && strcmp(password, ADMIN_PASSWORD) == 0) {
        fflush(stdout);
        fflush(stdin);
        send(connFD, ADMIN_LOGIN_SUCCESS, strlen(ADMIN_LOGIN_SUCCESS),0);
        printf("Sent the login success ");


        // Admin menu loop
        while (1) {
            fflush(stdout);
            fflush(stdin);
            send(connFD, ADMIN_MENU, strlen(ADMIN_MENU),0);
            printf("\n Sending from server : %s",ADMIN_MENU);
            //char choice[5];
            memset(client_message,'\0',sizeof(client_message));
            recv(connFD, client_message, sizeof(client_message),0);
            int t=atoi(client_message);
            printf("Received on server : %d",t);

            switch (t) {
                case 1:printf("I CAME HERE \n");addNewEmployee(connFD); break;
                case 3:printf("I CAME 2 \n");getEmployeeDetails(connFD); break;
                case 2: printf("I came 3 \n");modifyEmployeeDetails(connFD); break;
                // case 4: manageUserRoles(connFD); break;
                
                
                default: printf("Invalid \n");return;//adminLogout(connFD); return;
            }
        }
    }
     
    else {
        write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }
}
// Function to add a new employee
void addNewEmployee(int connFD) {
    struct Employee newEmployee;
    char successMessage[150];char input[100];
    char server_message[2000], client_message[2000];
    char promptName[] = "&Enter employee's name: \n";
    char promptRole[] = "&Enter employee's role (e.g., 1 for Manager,0 for employee): \n";
    char promptLoginID[] = "&Enter employee's login ID: \n";
    char promptPassword[] = "&Enter employee's password: \n";

    // Prompt for employee's name
    fflush(stdout);
    fflush(stdin);
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    strcpy(server_message, promptName);
    send(connFD, server_message,strlen(server_message),0);
    recv(connFD,client_message,sizeof(client_message),0);
    printf("\n Username received : %s \n", client_message);
    strcpy(newEmployee.name,client_message);

    

    char roleInput[10];  // Buffer to hold the user's input (as a string)

// Prompt for employee's role
    fflush(stdout);
    fflush(stdin);
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
    strcpy(server_message, promptRole);
    send(connFD, server_message,strlen(server_message),0);
    recv(connFD,client_message,sizeof(client_message),0);
    printf("\n Role received : %s \n", client_message);
    strcpy(input,client_message);
    

// Convert the string to an integer and assign it to newEmployee.type
    newEmployee.type = atoi(input);
    newEmployee.id=generate_id();
    strcpy(input,"");
    snprintf(input,sizeof(input),"%s-%d",newEmployee.name,newEmployee.id);
    //strcpy(input,)
    strcpy(newEmployee.password,input);


   FILE *file = fopen("employee.txt", "a"); // Open file in append mode
if (file == NULL) {
    const char *errorMessage = "*Error saving employee details.\n";
    write(connFD, errorMessage, strlen(errorMessage));
    return;
}

// Use fprintf to write employee details in CSV format
if (fprintf(file, "%s,%d,%d,%s\n", newEmployee.name, newEmployee.id, newEmployee.type, newEmployee.password) < 0) {
    const char *writeErrorMessage = "*Error writing employee details to the file.\n";
    write(connFD, writeErrorMessage, strlen(writeErrorMessage));
} else {
    // Prepare success message
    snprintf(successMessage, sizeof(successMessage), 
             "Employee %d added successfully with role %s.\n", 
             newEmployee.id, newEmployee.password);

    write(connFD, successMessage, strlen(successMessage));
}

fclose(file); 
}
    

void modifyEmployeeDetails(int connFD) {
    //struct Employee employee;
    char promptID[] = "&Enter the employee's ID to modify:\n";
    char employeeNotFound[] = "*Employee not found.\n";
    char modifyMenu[] = "&Modify Menu:\n1. Name\n2. Role\n3. Password\n4. Exit\nEnter your choice: \n";
    char successMessage[] = "*Employee details updated successfully.\n";
    char optionNotValid[] = "*Invalid option.\n";
    int searchID, choice;char server_message[200];

    // Prompt for employee ID
    fflush(stdin);
    fflush(stdout);
    write(connFD, promptID, strlen(promptID));
    printf("Sent menu \n");
    memset(server_message,'\0', sizeof(server_message));
    recv(connFD, server_message, sizeof(server_message),0);
    searchID=atoi(server_message);
    printf("I received %d \n",searchID);
    fflush(stdin);
    fflush(stdout);

    // Open the employee file
    FILE *file = fopen("employee.txt", "r+"); // Open the file for reading and updating in text mode
    //struct Employee employee;
    
    if (file == NULL) {
        const char *errorMessage = "*Error opening employee details file.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Temporary file for writing updated content
    FILE *tempFile = fopen("temp_employee.txt", "w+");
if (tempFile == NULL) {
    const char *errorMessage = "*Error opening temporary file.\n";
    write(connFD, errorMessage, strlen(errorMessage));
    fclose(file);
    return;
}

int found = 0;
char line[256]; // Buffer to hold each line from the file

// Read each employee record from the file in CSV format
while (fgets(line, sizeof(line), file)) {
    struct Employee employee;
    int choice;
    // Parse the CSV line and populate the employee struct
    if (sscanf(line, "%[^,],%d,%d,%[^,\n]", employee.name, &employee.id, &employee.type, employee.password) == 4) {
        if (employee.id == searchID) {
            found = 1; // Employee found
            char modifyMenu[] = "&Modify Options:\n1.Name\n2.Role\n3.Password\n4.Exit\nChoose an option:\n";
            
            // Send modify menu to client
            
            
                // Display modify menu to client
                
                
                send(connFD, modifyMenu, strlen(modifyMenu),0);
                memset(server_message,'\0',sizeof(server_message));
                recv(connFD, server_message, sizeof(server_message),0); // Read user choice
                choice=atoi(server_message);
                printf("I read choice %d \n",choice);

                switch (choice) {
                    case 1: 
                        // Modify Name
                        fflush(stdout);
                        char promptName[] = "&Enter new name:\n";
                        write(connFD, promptName, strlen(promptName));
                        bzero(employee.name, sizeof(employee.name)); // Clear previous name
                        read(connFD, employee.name, sizeof(employee.name)); // Read new name from client
                        employee.name[strcspn(employee.name, "\n")] = 0; // Remove newline character
                        break;
                    

                    case 2: 
                        // Modify Role
                        char promptRole[] = "&Enter new role (Manager=1, Employee=0):\n";
                        fflush(stdout);
                        send(connFD, promptRole, strlen(promptRole),0);
                        char roleInput[10];
                        memset(roleInput, '\0', sizeof(roleInput));
                        recv(connFD, roleInput, sizeof(roleInput),0);
                        employee.type = atoi(roleInput); // Convert role input to integer (1 or 0)
                        break;
                    

                    case 3: 
                        // Modify Password
                        fflush(stdout);
                        char promptPassword[] = "&Enter new password: \n";
                        write(connFD, promptPassword, strlen(promptPassword));
                        bzero(employee.password, sizeof(employee.password)); // Clear previous password
                        read(connFD, employee.password, sizeof(employee.password)); // Read new password
                        employee.password[strcspn(employee.password, "\n")] = 0; // Remove newline character
                        break;
                    

                    case 4:
                        // Exit the menu
                        fflush(stdout);
                        char successMessage[] = "*Exiting modification menu.\n";
                        write(connFD, successMessage, strlen(successMessage));
                        break;

                    default:
                        fflush(stdout);
                        printf("I entered here");
                        char optionNotValid[] = "*Invalid option. Try again.\n";
                        write(connFD, optionNotValid, strlen(optionNotValid));
                }

                // If a modification was made, update the record in the temp file
                if (choice >= 1 && choice <= 3) {
                    fprintf(tempFile, "%s,%d,%d,%s\n", employee.name, employee.id, employee.type, employee.password);
                }

            

        } else {
            // If this is not the searched employee, just write the record to the temp file
            fprintf(tempFile, "%s,%d,%d,%s\n", employee.name, employee.id, employee.type, employee.password);
        }
    }
}

if (!found) {
    const char *notFoundMessage = "*Employee ID not found.\n";
    fflush(stdout);
    write(connFD, notFoundMessage, strlen(notFoundMessage));
}

fclose(file);
fclose(tempFile);

// Replace the old file with the new one (only if modifications were made)
if (found) {
    remove("employee.txt");
    rename("temp_employee.txt", "employee.txt");
} else {
    remove("temp_employee.txt");
}
}

void getEmployeeDetails(int connFD) {
    struct Employee employee;
    char promptID[] = "&Enter the employee's ID:\n";
    char employeeNotFound[] = "*Employee not found.\n";
    //char employeeDetails[256];
    int searchID;char server_message[200];

    // Prompt for employee ID
    write(connFD, promptID, strlen(promptID));
    memset(server_message,'\0',sizeof(server_message));
    recv(connFD, server_message, sizeof(server_message),0);
    searchID=atoi(server_message);
    int fileFD = open("employee.txt", O_RDONLY);
    if (fileFD < 0) {
        const char *errorMessage = "Error opening employee details file.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }
    

    char line[500];  // Buffer to hold each line from the file
    char employeeDetails[256];    // Buffer to store formatted employee details
    int found = 0;                // Flag to check if employee is found

    // Read each line from the file
    while (read(fileFD, line, sizeof(line)) > 0) {
        struct Employee employee;  // Declare an employee structure
        char *token;

        // Parse the line using strtok to separate the fields
        token = strtok(line, ",");
        if (token != NULL) {
            strncpy(employee.name, token, sizeof(employee.name));
            employee.name[sizeof(employee.name) - 1] = '\0'; // Null-terminate

            token = strtok(NULL, ","); // Get ID
            if (token != NULL) {
                employee.id = atoi(token); // Convert string to integer
            }

            token = strtok(NULL, ","); // Get Type
            if (token != NULL) {
                employee.type = atoi(token); // Convert string to integer
            }

            token = strtok(NULL, ","); // Get Password
            if (token != NULL) {
                strncpy(employee.password, token, sizeof(employee.password));
                employee.password[sizeof(employee.password) - 1] = '\0'; // Null-terminate
            }

            // Check if the employee ID matches the search ID
            if (employee.id == searchID) {
                // Employee found, prepare details to send
                snprintf(employeeDetails, sizeof(employeeDetails),
                         "*Name: %s\nRole: %d\nID: %d\nType: %s\nPassword: %s\n\n", 
                         employee.name, employee.type, employee.id, 
                         employee.type == 1 ? "Manager" : "Employee", 
                         employee.password);
                write(connFD, employeeDetails, strlen(employeeDetails));
                found = 1; // Set found flag
                break; // Exit the loop since we found the employee
            }
        }
    }

    // If employee is not found
    if (!found) {
        //const char *employeeNotFound = "Employee not found.\n";
        write(connFD, employeeNotFound, strlen(employeeNotFound));
    }

    // Close the file descriptor
    close(fileFD);
}





// void manageUserRoles(int connFD) {
//     char loginID[100];
//     char newRole[20];
    
//     // Prompt for login ID
//     const char *promptLoginID = "Enter the login ID of the employee whose role you want to change: ";
//     write(connFD, promptLoginID, strlen(promptLoginID));
//     read(connFD, loginID, sizeof(loginID));
//     loginID[strcspn(loginID, "\n")] = 0; // Remove newline character

//     // Prompt for new role
//     const char *promptNewRole = "&Enter the new role for this employee (admin, manager, employee): ";
//     write(connFD, promptNewRole, strlen(promptNewRole));
//     read(connFD, newRole, sizeof(newRole));
//     newRole[strcspn(newRole, "\n")] = 0; // Remove newline character

//     // Validate new role
//     if (strcmp(newRole,"ADMIN") != 0 && strcmp(newRole,"MANAGER") != 0 && strcmp(newRole,"EMPLOYEE") != 0) {
//         const char *errorMessage = "Invalid role specified. Please use admin, manager, or employee.\n";
//         write(connFD, errorMessage, strlen(errorMessage));
//         return;
//     }

//     // Open the employee data file for reading and writing
//     int fileFD = open(EMPLOYEE_FILE, O_RDWR);
//     if (fileFD == -1) {
//         const char *errorMessage = "Error accessing employee data. Please try again later.\n";
//         write(connFD, errorMessage, strlen(errorMessage));
//         return;
//     }

//     char line[256];
//     char updatedLine[256];
//     int found = 0;

//     // Read through the file and change the role
//     while (read(fileFD, line, sizeof(line)) > 0) {
//         char empLoginID[100], empRole[20];
//         sscanf(line, "%[^,],%*[^,],%*[^,],%*d,%*[^,],%*[^,],%s", empLoginID, empRole);

//         if (strcmp(empLoginID, loginID) == 0) {
//             found = 1;
//             snprintf(updatedLine, sizeof(updatedLine), "%s,%s\n", loginID, newRole);
//             break;
//         }
//     }

//     // If employee found, update the role
//     if (found) {
//         // Seek to the start of the line for updating
//         lseek(fileFD, -strlen(line), SEEK_CUR);
//         write(fileFD, updatedLine, strlen(updatedLine));
//         const char *successMessage = "Employee role updated successfully.\n";
//         write(connFD, successMessage, strlen(successMessage));
//     } else {
//         const char *notFoundMessage = "No employee found with the specified login ID.\n";
//         write(connFD, notFoundMessage, strlen(notFoundMessage));
//     }

//     close(fileFD); // Close the file descriptor
// }

// void adminLogout(int connFD) {

// }
int generate_id() {
    struct Employee emp;
    int max_id = 0; // Variable to hold the maximum ID found
    FILE *file = fopen("employee.txt", "r"); // Open the file in read mode

    if (file == NULL) {
        perror("Unable to open file");
        return -1; // Return an error code if the file can't be opened
    }

    char line[256]; // Buffer to hold each line from the file

    // Read through the file to find the maximum ID using fgets and sscanf
    while (fgets(line, sizeof(line), file)) {
        // Parse the CSV line and populate the employee struct
        if (sscanf(line, "%[^,],%d,%d,%[^,\n]", emp.name, &emp.id, &emp.type, emp.password) == 4) {
            if (emp.id > max_id) {
                max_id = emp.id; // Update max_id if a larger ID is found
            }
        }
    }

    fclose(file);
    return max_id + 1; // Return the next ID
}

#endif
// Function to change the password of the admin
 