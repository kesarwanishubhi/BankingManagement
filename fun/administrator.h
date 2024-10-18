/*#include <stdbool.h>
#include <time.h>

typedef struct {
    int EmployeeID;                   // Unique identifier for each employee
    char FirstName[50];               // Employee's first name
    char LastName[50];                // Employee's last name
    char LoginID[50];                 // Unique login ID for the employee
    char Password[255];                // Password (hashed) for secure storage
    enum {
        TELLER,
        LOAN_OFFICER,
        BRANCH_MANAGER,
        CUSTOMER_SERVICE_REPRESENTATIVE,
        FINANCIAL_ANALYST,
        COMPLIANCE_OFFICER,
        RISK_MANAGER,
        INVESTMENT_BANKER,
        ACCOUNTANT,
        HR_MANAGER,
        
        IT_SPECIALIST,
        MORTGAGE_SPECIALIST,
        
        BRANCH_OPERATIONS_SPECIALIST
    } Role;                           // Role of the employee
    char Email[100];                  // Employee's email address
    char Phone[15];                   // Employee's phone number (optional)
    
    bool IsActive;                    // Status of employment (active/inactive)
    
} Employee;
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "server_constants.h"

// Function Prototypes
typedef struct {
    char name[100];
    char role[50];
    char loginID[100];
    char password[100];
} Employee;

void adminLogin(int connFD);
void addNewEmployee(int connFD)
void getAccountDetails(int connFD);
void manageUserRoles(int connFD);
void addAccount(int connFD);
void deleteAccount(int connFD);
void modifyCustomerInfo(int connFD);
void changePassword(int connFD);
void adminLogout(int connFD);

// Function to handle administrator login
void adminLogin(int connFD) {
    char loginID[100];
    char password[100];

    // Send welcome message to admin
    send(connFD, ADMIN_LOGIN_WELCOME, strlen(ADMIN_LOGIN_WELCOME), 0);
    
    // Get login ID from client
    send(connFD, LOGIN_ID, strlen(LOGIN_ID), 0);
    recv(connFD, loginID, sizeof(loginID), 0);

    // Get password from client
    send(connFD, PASSWORD, strlen(PASSWORD), 0);
    recv(connFD, password, sizeof(password), 0);

    // For demonstration purposes, using hardcoded admin credentials
    const char *correctLoginID = "admin";
    const char *correctPassword = "admin123";

    // Validate credentials
    if (strcmp(loginID, correctLoginID) == 0 && strcmp(password, correctPassword) == 0) {
        send(connFD, ADMIN_LOGIN_SUCCESS, strlen(ADMIN_LOGIN_SUCCESS), 0);
        
        // Admin menu loop
        while (1) {
            send(connFD, ADMIN_MENU, strlen(ADMIN_MENU), 0);
            int choice;
            recv(connFD, &choice, sizeof(choice), 0);
            
            switch (choice) {
                case 1: addNewEmployee(connFD); break;
                case 2: getAccountDetails(connFD); break;
                case 3: getTransactionDetails(connFD); break;
                case 4: manageUserRoles(connFD); break;
                case 5: deleteAccount(connFD); break;
                case 6: modifyCustomerInfo(connFD); break;
                case 7: changePassword(connFD); break;
                default: adminLogout(connFD); return;
            }
        }
    } else {
        send(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN), 0);
    }
}
// Function to add a new employee
void addNewEmployee(int connFD) {
    Employee newEmployee;
    char successMessage[150];
    char promptName[] = "Enter employee's name: ";
    char promptRole[] = "Enter employee's role (e.g., Manager, Teller): ";
    char promptLoginID[] = "Enter employee's login ID: ";
    char promptPassword[] = "Enter employee's password: ";

    // Prompt for employee's name
    write(connFD, promptName, strlen(promptName));
    read(connFD, newEmployee.name, sizeof(newEmployee.name));
    newEmployee.name[strcspn(newEmployee.name, "\n")] = 0; // Remove newline

    // Prompt for employee's role
    write(connFD, promptRole, strlen(promptRole));
    read(connFD, newEmployee.role, sizeof(newEmployee.role));
    newEmployee.role[strcspn(newEmployee.role, "\n")] = 0; // Remove newline

    // Prompt for employee's login ID
    write(connFD, promptLoginID, strlen(promptLoginID));
    read(connFD, newEmployee.loginID, sizeof(newEmployee.loginID));
    newEmployee.loginID[strcspn(newEmployee.loginID, "\n")] = 0; // Remove newline

    // Prompt for employee's password
    write(connFD, promptPassword, strlen(promptPassword));
    read(connFD, newEmployee.password, sizeof(newEmployee.password));
    newEmployee.password[strcspn(newEmployee.password, "\n")] = 0; // Remove newline

    // Open the file to save the new employee
    int fileFD = open("employees.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fileFD < 0) {
        const char *errorMessage = "Error saving employee details.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    // Prepare employee details to be written to the file
    char employeeData[256];
    snprintf(employeeData, sizeof(employeeData), "Name: %s\nRole: %s\nLogin ID: %s\nPassword: %s\n\n",
             newEmployee.name, newEmployee.role, newEmployee.loginID, newEmployee.password);

    // Write employee data to the file
    if (write(fileFD, employeeData, strlen(employeeData)) < 0) {
        const char *writeErrorMessage = "Error writing employee details to the file.\n";
        write(connFD, writeErrorMessage, strlen(writeErrorMessage));
    } else {
        // Prepare success message
        snprintf(successMessage, sizeof(successMessage), "Employee %s added successfully with role %s.\n", newEmployee.name, newEmployee.role);
        write(connFD, successMessage, strlen(successMessage));
    }

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
    const char *promptNewRole = "Enter the new role for this employee (admin, manager, employee): ";
    write(connFD, promptNewRole, strlen(promptNewRole));
    read(connFD, newRole, sizeof(newRole));
    newRole[strcspn(newRole, "\n")] = 0; // Remove newline character

    // Validate new role
    if (strcmp(newRole, ROLE_ADMIN) != 0 && strcmp(newRole, ROLE_MANAGER) != 0 && strcmp(newRole, ROLE_EMPLOYEE) != 0) {
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
// Function to change the password of the admin
void changePassword(int connFD) {
    char oldPassword[100];
    char newPassword[100];
    char confirmPassword[100];
    
    // Prompt for old password
    const char *promptOldPassword = PASSWORD_CHANGE_OLD_PASS;
    write(connFD, promptOldPassword, strlen(promptOldPassword));
    read(connFD, oldPassword, sizeof(oldPassword));
    oldPassword[strcspn(oldPassword, "\n")] = 0; // Remove newline character

    // Open the password file to verify the old password
    int fileFD = open(ADMIN_PASSWORD_FILE, O_RDONLY);
    if (fileFD == -1) {
        const char *errorMessage = "Error accessing password file. Please try again later.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    char storedPassword[100];
    read(fileFD, storedPassword, sizeof(storedPassword));
    storedPassword[strcspn(storedPassword, "\n")] = 0; // Remove newline character

    // Verify old password
    if (strcmp(oldPassword, storedPassword) != 0) {
        const char *invalidPasswordMessage = PASSWORD_CHANGE_OLD_PASS_INVALID;
        write(connFD, invalidPasswordMessage, strlen(invalidPasswordMessage));
        close(fileFD);
        return;
    }

    // Prompt for new password
    const char *promptNewPassword = PASSWORD_CHANGE_NEW_PASS;
    write(connFD, promptNewPassword, strlen(promptNewPassword));
    read(connFD, newPassword, sizeof(newPassword));
    newPassword[strcspn(newPassword, "\n")] = 0; // Remove newline character

    // Prompt to confirm new password
    const char *promptConfirmPassword = PASSWORD_CHANGE_NEW_PASS_RE;
    write(connFD, promptConfirmPassword, strlen(promptConfirmPassword));
    read(connFD, confirmPassword, sizeof(confirmPassword));
    confirmPassword[strcspn(confirmPassword, "\n")] = 0; // Remove newline character

    // Validate new password confirmation
    if (strcmp(newPassword, confirmPassword) != 0) {
        const char *passwordMismatchMessage = PASSWORD_CHANGE_NEW_PASS_INVALID;
        write(connFD, passwordMismatchMessage, strlen(passwordMismatchMessage));
        close(fileFD);
        return;
    }

    // Write the new password to the file
    fileFD = open(ADMIN_PASSWORD_FILE, O_WRONLY | O_TRUNC);
    if (fileFD == -1) {
        const char *errorMessage = "Error accessing password file. Please try again later.\n";
        write(connFD, errorMessage, strlen(errorMessage));
        return;
    }

    write(fileFD, newPassword, strlen(newPassword));
    close(fileFD);

    const char *successMessage = PASSWORD_CHANGE_SUCCESS;
    write(connFD, successMessage, strlen(successMessage));
}
