//
// Created by fegrue on 21.04.23.
//
#include <stdio.h>
#include "stdlib.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "osmprun.h"


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void forker(int nprocesses, pid_t pid[])
{
    pid[nprocesses];

    if(nprocesses > 0)
    {
        if ((pid[nprocesses] = fork()) < 0)
        {
            perror("fork");
        }
        else if (pid[nprocesses] == 0)
        {
            //Child stuff here
            execvp("./output/echoall", "");
        }
        else if(pid[nprocesses] > 0)
        {
            waitpid(pid[nprocesses], NULL, WNOHANG);
            //parent
            forker(nprocesses - 1, pid);
        }
    }else{
        printf("%d", sizeof(*pid));
        for (int i; i< sizeof(*pid); i++)
        ;
    }

}

int main(int argv, char* argc[]) {

    int i = atol(argc[1]);
    pid_t pid[i];
    forker(i, pid);
    for(int j = 0 ; j <= i ; j++){
        printf("Wait for pid %d\n", pid[j]);
        waitpid(pid[j], NULL, WNOHANG);
    }
}