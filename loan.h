#ifndef LOAN_RECORD
#define LOAN_RECORD

#include <time.h>
#define LOAN_PURPOSE_MAX_LEN 50
struct Loan {
    int loanID;                    // Unique loan ID
    int accountNumber;             // Account number of the customer applying for the loan
    long int loanAmount;           // Loan amount being requested
    int customerID;                //customer id
    int isAssigned;                // 1 for assigned 0 for not assigned
    char loanPurpose[LOAN_PURPOSE_MAX_LEN]; // Purpose of the loan
    char status[15];                   // status of loan
    time_t applicationDate;        // Date and time of loan application
};



#endif // LOAN_RECORD
