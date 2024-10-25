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


void adminLogin(int connFD) {
    char loginID[100];
    char password[100];

    // Send welcome message to admin
    //write(connFD, ADMIN_LOGIN_WELCOME, strlen(ADMIN_LOGIN_WELCOME));

    // Get login ID from client
    fflush(stdout);
        fflush(stdin);
    write(connFD, LOGIN_ID, strlen(LOGIN_ID));
    bzero(loginID, sizeof(loginID));
    read(connFD, loginID, sizeof(loginID));
    //loginID[strcspn(loginID, "\n")] = '\0';
    printf("Username received : %s \n", loginID);

    // Get password from client
    fflush(stdout);
        fflush(stdin);
    write(connFD, PASSWORD, strlen(PASSWORD));
    bzero(password, sizeof(password));
    read(connFD, password, sizeof(password));
    //password[strcspn(password, "\n")] = '\0'; // Remove newline character if any
     printf("Password received : %s \n", password);


    if (strcmp(loginID, ADMIN_LOGIN_ID) == 0 && strcmp(password, ADMIN_PASSWORD) == 0) {
        write(connFD, ADMIN_LOGIN_SUCCESS, strlen(ADMIN_LOGIN_SUCCESS));

        // Admin menu loop
        while (1) {
            write(connFD, ADMIN_MENU, strlen(ADMIN_MENU));
            int choice;
            bzero(&choice, sizeof(choice));
            read(connFD, &choice, sizeof(choice));

            switch (choice) {
                case 1: addNewEmployee(connFD); break;
                case 2: getEmployeeDetails(connFD); break;
                case 3: modifyEmployeeDetails(connFD); break;
                case 4: manageUserRoles(connFD); break;
                case 6: modifyCustomerInfo(connFD); break;
                
                default: adminLogout(connFD); return;
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
    char successMessage[150];
    char promptName[] = "&Enter employee's name: ";
    char promptRole[] = "&Enter employee's role (e.g., Manager, Teller): ";
    char promptLoginID[] = "&Enter employee's login ID: ";
    char promptPassword[] = "&Enter employee's password: ";

    // Prompt for employee's name
    write(connFD, promptName, strlen(promptName));
    bzero(newEmployee.name, sizeof(newEmployee.name));
    read(connFD, newEmployee.name, sizeof(newEmployee.name));
    newEmployee.name[strcspn(newEmployee.name, "\n")] = 0; // Remove newline

    char roleInput[10];  // Buffer to hold the user's input (as a string)

// Prompt for employee's role
    write(connFD, promptRole, strlen(promptRole));
    bzero(roleInput, sizeof(roleInput));
    read(connFD, roleInput, sizeof(roleInput));
    roleInput[strcspn(roleInput, "\n")] = 0;  // Remove newline

// Convert the string to an integer and assign it to newEmployee.type
    newEmployee.type = atoi(roleInput);

    char input[10];
    // Prompt for employee's login ID
    write(connFD, promptLoginID, strlen(promptLoginID));
    bzero(input, sizeof(input));
    read(connFD, input, sizeof(input));
    newEmployee.id = atoi(input);
    // newEmployee.id[strcspn(newEmployee.id, "\n")] = 0; // Remove newline

    // Prompt for employee's password
    write(connFD, promptPassword, strlen(promptPassword));
    bzero(newEmployee.password, sizeof(newEmployee.password));
    read(connFD, newEmployee.password, sizeof(newEmployee.password));
    newEmployee.password[strcspn(newEmployee.password, "\n")] = 0; // Remove newline

    // Open the file to save the new employee
    int fileFD = open("EMPLOYEE_FILE", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fileFD < 0) {
        const char *errorMessage = "*Error saving employee details.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Prepare employee details to be written to the file
    char employeeData[256];
    snprintf(employeeData, sizeof(employeeData), 
             "Name: %s\nRole: %d\nLogin ID: %d\nPassword: %s\n\n", 
             newEmployee.name, newEmployee.type, newEmployee.id, newEmployee.password);

    // Write employee data to the file
    if (write(fileFD, employeeData, strlen(employeeData)) < 0) {
        const char *writeErrorMessage = "Error writing employee details to the file.\n";
        write(connFD, writeErrorMessage, strlen(writeErrorMessage));
    } else {
        // Prepare success message
        snprintf(successMessage, sizeof(successMessage), 
                 "Employee %d added successfully with role %s.\n", 
                 newEmployee.id, newEmployee.password);
        write(connFD, successMessage, strlen(successMessage));
    }

    // Close the file descriptor
    close(fileFD);
}
void modifyEmployeeDetails(int connFD) {
    struct Employee employee;
    char promptID[] = "&Enter the employee's ID to modify: ";
    char employeeNotFound[] = "*Employee not found.\n";
    char modifyMenu[] = "&Modify Menu:\n1. Name\n2. Role\n3. Password\n4. Exit\nEnter your choice: ";
    char successMessage[] = "*Employee details updated successfully.\n";
    char optionNotValid[] = "*Invalid option.\n";
    int searchID, choice;

    // Prompt for employee ID
    write(connFD, promptID, strlen(promptID));
    bzero(&searchID, sizeof(searchID));
    read(connFD, &searchID, sizeof(searchID));

    // Open the employee file
    int fileFD = open("EMPLOYEE_FILE", O_RDWR);
    if (fileFD < 0) {
        const char *errorMessage = "*Error opening employee details file.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Search for the employee by ID
    off_t position;
    while ((position = lseek(fileFD, 0, SEEK_CUR)) >= 0 && read(fileFD, &employee, sizeof(struct Employee)) > 0) {
        if (employee.id == searchID) {
            // Employee found, loop for modifying multiple fields
            do {
                // Send modify menu to client
                write(connFD, modifyMenu, strlen(modifyMenu));
                bzero(&choice, sizeof(choice));
                read(connFD, &choice, sizeof(choice));

                switch (choice) {
                    case 1: {
                        // Modify Name
                        char promptName[] = "&Enter new name: ";
                        write(connFD, promptName, strlen(promptName));
                        bzero(employee.name, sizeof(employee.name));
                        read(connFD, employee.name, sizeof(employee.name));
                        employee.name[strcspn(employee.name, "\n")] = 0;  // Remove newline
                        break;
                    }

                    case 2: {
                        // Modify Role
                        char promptRole[] = "&Enter new role (Manager,EMPLOYEE): ";
                        write(connFD, promptRole, strlen(promptRole));
                        char roleInput[10];
                        bzero(roleInput, sizeof(roleInput));
                        read(connFD, roleInput, sizeof(roleInput));
                        roleInput[strcspn(roleInput, "\n")] = '\0';
                        employee.type = atoi(roleInput);
                        // employee.type[strcspn(employee.type, "\n")] = 0;  // Remove newline

                        // Set employee type based on role
                        // if (employee.type == 1) {
                        //     employee.type = 1;  // Manager
                        // } else {
                        //     employee.type = 0;  // Non-manager
                        // }
                        break;
                    }

                    case 3: {
                        // Modify Password
                        char promptPassword[] = "&Enter new password: ";
                        write(connFD, promptPassword, strlen(promptPassword));
                        bzero(employee.password, sizeof(employee.password));
                        read(connFD, employee.password, sizeof(employee.password));
                        employee.password[strcspn(employee.password, "\n")] = 0;  // Remove newline
                        break;
                    }

                    case 4:
                        // Exit the menu
                        write(connFD, successMessage, strlen(successMessage));
                        break;

                    default:
                        write(connFD, optionNotValid, strlen(optionNotValid));
                }

                // Update the file after each modification (unless the user exits)
                if (choice >= 1 && choice <= 3) {
                    // Seek back to the position of the employee entry to overwrite it
                    lseek(fileFD, position, SEEK_SET);
                    if (write(fileFD, &employee, sizeof(struct Employee)) < 0) {
                        const char *writeErrorMessage = "*Error writing updated employee details to the file.\n";
                        write(connFD, writeErrorMessage, strlen(writeErrorMessage));
                        close(fileFD);
                        return;
                    }
                }

            } while (choice != 4);  // Exit loop when the user chooses option 4

            close(fileFD);
            return;
        }
    }

    // If employee is not found
    write(connFD, employeeNotFound, strlen(employeeNotFound));

    // Close the file descriptor
    close(fileFD);
}
void getEmployeeDetails(int connFD) {
    struct Employee employee;
    char promptID[] = "&Enter the employee's ID: ";
    char employeeNotFound[] = "*Employee not found.\n";
    char employeeDetails[256];
    int searchID;

    // Prompt for employee ID
    write(connFD, promptID, strlen(promptID));
    bzero(&searchID, sizeof(searchID));
    read(connFD, &searchID, sizeof(searchID));

    // Open the employee file
    int fileFD = open("EMPLOYEE_FILE", O_RDONLY);
    if (fileFD < 0) {
        const char *errorMessage = "Error opening employee details file.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Read and search through the file for the requested employee
    while (read(fileFD, &employee, sizeof(struct Employee)) > 0) {
        if (employee.id == searchID) {
            // Employee found, prepare details to send
            snprintf(employeeDetails, sizeof(employeeDetails),
                     "*Name: %s\nRole: %d\nID: %d\nType: %s\nPassword: %s\n\n", 
                     employee.name, employee.type, employee.id, 
                     employee.type == 1 ? "Manager" : "Employee", 
                     employee.password);
            write(connFD, employeeDetails, strlen(employeeDetails));
            close(fileFD);
            return;
        }
    }

    // If employee is not found
    write(connFD, employeeNotFound, strlen(employeeNotFound));

    // Close the file descriptor
    close(fileFD);
}
void modifyCustomerInfo(int connFD) {
    struct Customer customer;
    char promptID[] = "&Enter the customer's ID to modify: ";
    char customerNotFound[] = "*Customer not found.\n";
    char modifyMenu[] = "&Modify Menu:\n1. Name\n2. Gender\n3. Age\n4. Password\n5. Account Number\n6. Exit\nEnter your choice: ";
    char successMessage[] = "*Customer details updated successfully.\n";
    char optionNotValid[] = "*Invalid option.\n";
    int searchID, choice;

    // Prompt for customer ID
    write(connFD, promptID, strlen(promptID));
    bzero(&searchID, sizeof(searchID));
    read(connFD, &searchID, sizeof(searchID));

    // Open the customer file
    int fileFD = open("CUSTOMER_FILE", O_RDWR);
    if (fileFD < 0) {
        const char *errorMessage = "*Error opening customer details file.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Search for the customer by ID
    off_t position;
    while ((position = lseek(fileFD, 0, SEEK_CUR)) >= 0 && read(fileFD, &customer, sizeof(struct Customer)) > 0) {
        if (customer.id == searchID) {
            // Customer found, loop for modifying multiple fields
            do {
                // Send modify menu to client
                write(connFD, modifyMenu, strlen(modifyMenu));
                bzero(&choice, sizeof(choice));
                read(connFD, &choice, sizeof(choice));

                switch (choice) {
                    case 1: {
                        // Modify Name
                        char promptName[] = "&Enter new name: ";
                        write(connFD, promptName, strlen(promptName));
                        bzero(customer.name, sizeof(customer.name));
                        read(connFD, customer.name, sizeof(customer.name));
                        customer.name[strcspn(customer.name, "\n")] = 0;  // Remove newline
                        break;
                    }

                    case 2: {
                        // Modify Gender
                        char promptGender[] = "&Enter gender (M/F/O): ";
                        write(connFD, promptGender, strlen(promptGender));
                        bzero(&customer.gender, sizeof(customer.gender));
                        read(connFD, &customer.gender, sizeof(customer.gender));
                        break;
                    }

                    case 3: {
                        // Modify Age
                        char promptAge[] = "&Enter new age: ";
                        write(connFD, promptAge, strlen(promptAge));
                        bzero(&customer.age, sizeof(customer.age));
                        read(connFD, &customer.age, sizeof(customer.age));
                        break;
                    }

                    case 4: {
                        // Modify Password
                        char promptPassword[] = "&Enter new password: ";
                        write(connFD, promptPassword, strlen(promptPassword));
                        bzero(customer.password, sizeof(customer.password));
                        read(connFD, customer.password, sizeof(customer.password));
                        customer.password[strcspn(customer.password, "\n")] = 0;  // Remove newline
                        break;
                    }

                    case 5: {
                        // Modify Account Number
                        char promptAccount[] = "&Enter new account number: ";
                        write(connFD, promptAccount, strlen(promptAccount));
                        char input[10];
                        bzero(input, sizeof(input));
                        read(connFD, input, sizeof(input));
                        input[strcspn(input, "\n")] = '\0';
                        customer.account = atoi(input);
                        break;
                    }

                    case 6:
                        // Exit the menu
                        write(connFD, successMessage, strlen(successMessage));
                        break;

                    default:
                        write(connFD, optionNotValid, strlen(optionNotValid));
                }

                // Update the file after each modification (unless the user exits)
                if (choice >= 1 && choice <= 5) {
                    // Seek back to the position of the customer entry to overwrite it
                    lseek(fileFD, position, SEEK_SET);
                    if (write(fileFD, &customer, sizeof(struct Customer)) < 0) {
                        const char *writeErrorMessage = "*Error writing updated customer details to the file.\n";
                        write(connFD, writeErrorMessage, strlen(writeErrorMessage));
                        close(fileFD);
                        return;
                    }
                }

            } while (choice != 6);  // Exit loop when the user chooses option 6

            close(fileFD);
            return;
        }
    }

    // If customer is not found
    write(connFD, customerNotFound, strlen(customerNotFound));

    // Close the file descriptor
    close(fileFD);
}






void manageUserRoles(int connFD) {
    char loginID[100];
    char newRole[20];
    
    // Prompt for login ID
    const char *promptLoginID = "Enter the login ID of the employee whose role you want to change: ";
    write(connFD, promptLoginID, strlen(promptLoginID));
    read(connFD, loginID, sizeof(loginID));
    loginID[strcspn(loginID, "\n")] = 0; // Remove newline character

    // Prompt for new role
    const char *promptNewRole = "&Enter the new role for this employee (admin, manager, employee): ";
    write(connFD, promptNewRole, strlen(promptNewRole));
    read(connFD, newRole, sizeof(newRole));
    newRole[strcspn(newRole, "\n")] = 0; // Remove newline character

    // Validate new role
    if (strcmp(newRole,"ADMIN") != 0 && strcmp(newRole,"MANAGER") != 0 && strcmp(newRole,"EMPLOYEE") != 0) {
        const char *errorMessage = "Invalid role specified. Please use admin, manager, or employee.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Open the employee data file for reading and writing
    int fileFD = open(EMPLOYEE_FILE, O_RDWR);
    if (fileFD == -1) {
        const char *errorMessage = "Error accessing employee data. Please try again later.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    char line[256];
    char updatedLine[256];
    int found = 0;

    // Read through the file and change the role
    while (read(fileFD, line, sizeof(line)) > 0) {
        char empLoginID[100], empRole[20];
        sscanf(line, "%[^,],%*[^,],%*[^,],%*d,%*[^,],%*[^,],%s", empLoginID, empRole);

        if (strcmp(empLoginID, loginID) == 0) {
            found = 1;
            snprintf(updatedLine, sizeof(updatedLine), "%s,%s\n", loginID, newRole);
            break;
        }
    }

    // If employee found, update the role
    if (found) {
        // Seek to the start of the line for updating
        lseek(fileFD, -strlen(line), SEEK_CUR);
        write(fileFD, updatedLine, strlen(updatedLine));
        const char *successMessage = "Employee role updated successfully.\n";
        write(connFD, successMessage, strlen(successMessage));
    } else {
        const char *notFoundMessage = "No employee found with the specified login ID.\n";
        write(connFD, notFoundMessage, strlen(notFoundMessage));
    }

    close(fileFD); // Close the file descriptor
}

void adminLogout(int connFD) {

}

#endif
// Function to change the password of the admin
 