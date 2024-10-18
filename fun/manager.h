#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <stdio.h>
#include <fcntl.h>    // For open()
#include <unistd.h>   // For close(), read(), write()
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>



// Account structure definition
typedef struct {
    int accountNumber;
    char accountHolderName[50];
    double balance;
    int isActive; // 0 for deactivated, 1 for activated
} Account;

typedef struct {
    int loanApplicationID;
    int accountNumber;
    char loanPurpose[100];
    double loanAmount;
    int isAssigned; // 0 for not assigned, 1 for assigned
    int assignedEmployeeID; // Holds the ID of the assigned employee
} LoanApplication;

// Function to check manager credentials
int checkManagerCredentials(const char* loginID, const char* password) {
    // In a real application, these would be checked against a database or file
    const char* correctLoginID = "admin";  // Example login ID
    const char* correctPassword = "admin123";  // Example password

    return (strcmp(loginID, correctLoginID) == 0 && strcmp(password, correctPassword) == 0);
}

// Function for manager login
void managerLogin() {
    char loginID[100];
    char password[100];
    int loginAttempts = 0;  // Initialize login attempts counter

    printf(MANAGER_LOGIN_WELCOME);

    while (loginAttempts < MAX_LOGIN_ATTEMPTS) {
        // Get manager login ID
        printf("%s\n", LOGIN_ID);
        scanf("%s", loginID);

        // Get manager password
        printf("%s", PASSWORD);
        scanf("%s", password);

        // Validate credentials
        if (checkManagerCredentials(loginID, password)) {
            printf(MANAGER_LOGIN_SUCCESS);
            // Proceed to the manager menu
            int choice;
            while (1) {
                printf(MANAGER_MENU);
                scanf("%d", &choice);
                switch (choice) {
                    case 1: // Activate/Deactivate Customer Account
                        activateAccount();
                        break;
                    case 2: // Assign Loan Applications
                        assignLoanApplication();
                        break;
                    case 3: // Review Customer Feedback
                        reviewCustomerFeedback();  // Call the feedback review function
                        break;
                    case 4: // Change Password
                        changeEmployeePassword(const char* loginID)
                        break;
                    case 5:
                        deactivateAccount();
                    case 6:
                        logout();
                    default:
                        printf(MANAGER_LOGOUT);
                        return; // Exit the manager session
                }
            }
        } else {
            loginAttempts++;  // Increment login attempts
            if (loginAttempts >= MAX_LOGIN_ATTEMPTS) {
                printf(LOGIN_ATTEMPT_EXCEEDED);
                return;  // Exit if attempts exceeded
            }
            printf(INVALID_LOGIN);  // Inform about invalid login
        }
    }
}

int main() {
    managerLogin();  // Start the login process for the manager
    return 0;
}


// Function to activate a customer account using system calls
void activateAccount() {
    int accountNumber, found = 0;
    Account account;
    ssize_t bytesRead;

    printf(MANAGER_ACTIVATE_ACCOUNT_PROMPT);
    scanf("%d", &accountNumber);

    // Open the account file for reading and writing
    int file = open(ACCOUNT_FILE, O_RDWR);
    if (file < 0) {
        perror("Error opening account file");
        return;
    }

    // Create a temporary file for writing the updated accounts
    int tempFile = open("temp.bank", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tempFile < 0) {
        perror("Error opening temporary file");
        close(file);
        return;
    }

    // Read each account from the file
    while ((bytesRead = read(file, &account, sizeof(Account))) > 0) {
        if (account.accountNumber == accountNumber) {
            found = 1;

            if (account.isActive == 1) {
                printf("Account is already active.\n");
            } else {
                // Activate the account
                account.isActive = 1;
                printf(MANAGER_ACTIVATE_SUCCESS);
            }
        }

        // Write the account (modified or unmodified) to the temporary file
        if (write(tempFile, &account, sizeof(Account)) != sizeof(Account)) {
            perror("Error writing to temporary file");
        }
    }

    if (!found) {
        printf(ACCOUNT_ID_DOESNT_EXIT);
    }

    // Close the files
    close(file);
    close(tempFile);

    // Replace the old file with the updated file
    if (remove(ACCOUNT_FILE) != 0) {
        perror("Error deleting old account file");
        return;
    }

    if (rename("temp.bank", ACCOUNT_FILE) != 0) {
        perror("Error renaming temporary file");
    }
}
void deactivateAccount() {
    int accountNumber, found = 0;
    Account account;
    ssize_t bytesRead;

    printf(MANAGER_DEACTIVATE_ACCOUNT_PROMPT);
    scanf("%d", &accountNumber);

    // Open the account file for reading and writing
    int file = open(ACCOUNT_FILE, O_RDWR);
    if (file < 0) {
        perror("Error opening account file");
        return;
    }

    // Create a temporary file for writing the updated accounts
    int tempFile = open("temp.bank", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tempFile < 0) {
        perror("Error opening temporary file");
        close(file);
        return;
    }

    // Read each account from the file
    while ((bytesRead = read(file, &account, sizeof(Account))) > 0) {
        if (account.accountNumber == accountNumber) {
            found = 1;

            if (account.isActive == 0) {
                printf(ACCOUNT_ALREADY_DEACTIVATED);
            } else {
                // Deactivate the account
                account.isActive = 0;
                printf(MANAGER_DEACTIVATE_SUCCESS);
            }
        }

        // Write the account (modified or unmodified) to the temporary file
        if (write(tempFile, &account, sizeof(Account)) != sizeof(Account)) {
            perror("Error writing to temporary file");
        }
    }

    if (!found) {
        printf(ACCOUNT_ID_DOESNT_EXIT);
    }

    // Close the files
    close(file);
    close(tempFile);

    // Replace the old file with the updated file
    if (remove(ACCOUNT_FILE) != 0) {
        perror("Error deleting old account file");
        return;
    }

    if (rename("temp.bank", ACCOUNT_FILE) != 0) {
        perror("Error renaming temporary file");
    }
}
void assignLoanApplication() { // not correct ask from someone
    int loanFile;
    int tempFile;
    LoanApplication loanApp;
    int employeeID;
    int found = 0;
    ssize_t bytesRead;

    // Prompt for employee ID
    printf(MANAGER_ASSIGN_LOAN_PROMPT);
    scanf("%d", &employeeID);

    // Open the loan application file for reading
    loanFile = open(LOAN_APPLICATION_FILE, O_RDONLY);
    if (loanFile == -1) {
        perror("Error opening loan application file");
        return;
    }

    // Open a temporary file for writing
    tempFile = open("temp_loan.bank", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tempFile == -1) {
        perror("Error opening temporary file");
        close(loanFile);
        return;
    }

    // Read through the loan application file
    while ((bytesRead = read(loanFile, &loanApp, sizeof(LoanApplication))) > 0) {
        if (loanApp.isAssigned == 0) {
            // Assign the loan to the employee
            loanApp.isAssigned = 1;
            loanApp.assignedEmployeeID = employeeID;
            found = 1;
            printf(MANAGER_ASSIGN_LOAN_SUCCESS);
        }

        // Write the loan application data to the temporary file
        if (write(tempFile, &loanApp, sizeof(LoanApplication)) == -1) {
            perror("Error writing to temporary file");
            close(loanFile);
            close(tempFile);
            return;
        }
    }

    if (!found) {
        printf(NO_LOAN_APPLICATIONS); // No loan applications available
    }

    // Close the files
    close(loanFile);
    close(tempFile);

    // Replace the old file with the new one
    if (unlink(LOAN_APPLICATION_FILE) == -1) {
        perror("Error deleting original loan application file");
        return;
    }
    if (rename("temp_loan.bank", LOAN_APPLICATION_FILE) == -1) {
        perror("Error renaming temporary file to loan application file");
    }
}
void reviewCustomerFeedback() {
    int feedbackFile;
    Feedback feedback;
    int bytesRead;
    int feedbackCount = 0;

    // Open the feedback file for reading
    feedbackFile = open(FEEDBACK_FILE, O_RDONLY);
    if (feedbackFile == -1) {
        perror("Error opening feedback file");
        return;
    }

    printf(MANAGER_REVIEW_FEEDBACK_PROMPT);

    // Read each feedback from the file
    while ((bytesRead = read(feedbackFile, &feedback, sizeof(Feedback))) > 0) {
        feedbackCount++;
        printf("Feedback ID: %d\n", feedback.feedbackID);
        printf("Account Number: %d\n", feedback.accountNumber);
        printf("Feedback: %s\n", feedback.feedbackText);
        printf("Date: %s", ctime(&feedback.feedbackDate)); // Convert to readable format
        printf("--------------------------------------------------\n");
    }

    if (bytesRead == -1) {
        perror("Error reading feedback file");
    } else if (feedbackCount == 0) {
        printf(NO_FEEDBACKS_AVAILABLE); // No feedbacks available
    }

    // Close the feedback file
    close(feedbackFile);
}
int changeEmployeePassword(const char* loginID) {
    char oldPassword[100];
    char newPassword[100];
    char confirmPassword[100];

    // Get old password
    printf(PASSWORD_CHANGE_OLD_PASS);
    scanf("%s", oldPassword);

    // Check if the old password is correct
    for (int i = 0; i < numEmployees; i++) {
        if (strcmp(employees[i].loginID, loginID) == 0 && strcmp(employees[i].password, oldPassword) == 0) {
            // Get new password
            printf(PASSWORD_CHANGE_NEW_PASS);
            scanf("%s", newPassword);

            // Confirm new password
            printf(PASSWORD_CHANGE_NEW_PASS_RE);
            scanf("%s", confirmPassword);

            if (strcmp(newPassword, confirmPassword) == 0) {
                // Change the password
                strcpy(employees[i].password, newPassword);
                printf(PASSWORD_CHANGE_SUCCESS);
                return 1; // Password change successful
            } else {
                printf(PASSWORD_CHANGE_NEW_PASS_INVALID);
                return 0; // Passwords do not match
            }
        }
    }

    printf(PASSWORD_CHANGE_OLD_PASS_INVALID);
    return 0; // Old password is incorrect
}






