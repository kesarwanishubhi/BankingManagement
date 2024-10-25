#ifndef ACCOUNT_RECORD
#define ACCOUNT_RECORD

#define MAX_TRANSACTIONS 10

struct Account
{
    int accountNumber;     // 0, 1, 2, ....
    int customerid;         // Customer IDs
    
    int active;           // 1 -> Active, 0 -> Deactivated (Deleted)
    long int balance;      // Amount of money in the account
    int transactionCount; // Number of transactions associated with the account
    //struct Transaction transactions[MAX_TRANSACTIONS];
    //int transactions[MAX_TRANSACTIONS];  // A list of transaction IDs. Used to look up the transactions. // -1 indicates unused space in array
};

#endif