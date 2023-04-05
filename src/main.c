#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


// Authors: Felix Grüning, Patrick Zockol
// Last Change: 05.04.2023
// Task: 1. Praktikumstermin | UNIX Prozessmanagement


#define CHILD 1

int main(int argv, char* argc[]){

    printf("STRERROR_START: %s\n", strerror(errno));

    pid_t pid = fork();
    // Kindprozess bei PID = 0
    // Elternprozess da PID = XXXXX und XXXXX ist die PID vom Kind
    int i;
    for (i = 0; i < 3; i++) {

        if (pid != 0) {
            printf("PID = %d\n", pid);
            waitpid(pid, system, NULL);
            printf("Elternprozess ab hier\n");
        } else if (pid == 0) {
            printf("Kindprozess\n");
            execlp("./output/echoall", "echoall", "Echoall" ,"Test", NULL);

        }
        sleep(3);
    }


    return 0;

}