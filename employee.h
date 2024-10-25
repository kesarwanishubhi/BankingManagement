#ifndef EMPLOYEE_CREDENTIALS
#define EMPLOYEE_CREDENTIALS

// #define EMPLOYEE_LOGIN_ID "Narayan"
// #define EMPLOYEE_PASSWORD "narayan" // "420boo69"
#define MAX_LOANS_PER_EMPLOYEE 10
struct Employee{
	char name[50];
	//char email[50];
	int id; //employee id
	int type;  //1--> manager 0---->employee
	//int l[10]; //array of id's of loan
	char password[50]; //password of the employee
};

#endif