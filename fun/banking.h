struct Customer

Although the actual definition of struct Customer is not included in the provided code, we can deduce its structure based on its usage in the functions. The following fields are likely included:

    ID (int id):
        This is used to uniquely identify a customer. It is referenced in the get_customer_details function where it checks the customer ID.

    Name (char name[100]):
        This stores the customer's name. It is used in the output format string when displaying customer details.

    Gender (char gender):
        A character representing the gender of the customer. It is also displayed in the customer details.

    Age (int age):
        This field likely stores the age of the customer and is used in the sprintf function for displaying customer details.

    Account Number (int account):
        This may refer to the primary account linked to the customer. It is used when displaying the customer details.

    Login ID (char login[100]):
        This field stores the login ID used for authentication. The login_handler function uses it to verify user login.

    Password (char password[100]):
        This is likely to store the hashed password for security purposes, as it is checked during login authentication.

    Additional Fields (Possible):
        While the above fields are specifically referenced in the code, there may be additional fields related to the customer's contact information, address, or other personal details not explicitly mentioned in the current functions.
// loan.h

struct LoanApplication {
    int id;                    // Unique Loan Application ID
    int customerID;             // The ID of the customer applying for the loan
    long amount;                // The amount of the loan
    char purpose[100];          // The purpose of the loan
    char status[10];            // Status: "pending", "approved", "rejected"
};
        