#ifndef FEEDBACK_RECORD
#define FEEDBACK_RECORD

struct Feedback {
    int feedbackID;             // Unique ID for the feedback
    int accountNumber;          // Associated account number
    char feedbackText[500];     // Feedback message (adjust size as needed)
    time_t feedbackDate;        // Date and time when the feedback was submitted
};

#endif
