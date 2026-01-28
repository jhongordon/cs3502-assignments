#include <stdio.h>
#include <time.h>


int main() {
FILE *logfile;
char message[100];
time_t current_time;


// Open file in append mode
logfile = fopen("owltech.log", "a");


// Check if file opened successfully
if (logfile == NULL) {
printf("Error: Could not open log file\n");
return 1;
}


printf("Enter log message: ");
fgets(message, sizeof(message), stdin);


// Get current time
time(&current_time);


// Write timestamp and message to file
fprintf(logfile, "[%s] %s", ctime(&current_time), message);


// Close the file
fclose(logfile);


printf("Log entry saved!\n");
return 0;
}
