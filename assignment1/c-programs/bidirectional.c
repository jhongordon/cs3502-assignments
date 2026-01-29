#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int parent_to_child[2];
    int child_to_parent[2];
    pid_t pid;

    char parent_msg[] = "Hello from parent";
    char child_msg[]  = "Hello from child";
    char buffer[100];

    // Create both pipes
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Close unused ends
        close(parent_to_child[1]); // child doesn't write to this pipe
        close(child_to_parent[0]); // child doesn't read from this pipe

        // Read message from parent
        read(parent_to_child[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);

        // Send response to parent
        write(child_to_parent[1], child_msg, strlen(child_msg) + 1);

        // Close used ends
        close(parent_to_child[0]);
        close(child_to_parent[1]);
    } 
    else {
        // Close unused ends
        close(parent_to_child[0]); // parent doesn't read from this pipe
        close(child_to_parent[1]); // parent doesn't write to this pipe

        // Send message to child
        write(parent_to_child[1], parent_msg, strlen(parent_msg) + 1);

        // Read response from child
        read(child_to_parent[0], buffer, sizeof(buffer));
        printf("Parent received: %s\n", buffer);

        // Close used ends
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        wait(NULL);
    }

    return 0;
}
