#ifndef RECORD
#define RECORD

struct AssignedLoan {
    int loanID;                // Unique ID for the loan
    int accountNumber;         // Account number associated with the loan
    long loanAmount;           // Amount of the loan
    int customerID;            // Unique ID of the customer
    int employeeID;            // Employee ID assigned to handle this loan
    char loanPurpose[50];      // Purpose of the loan
    int status;                // 0-> for rejected and 1 for accepted
};
#endif