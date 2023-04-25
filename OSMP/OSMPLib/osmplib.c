#include <unistd.h>
#include <stdio.h>
#define SharedMemSize 100
#define SharedMemName "/shm"

int counter = 0;

int OSMP_Init(int *argc, char ***argv) {
    int fileDescriptor = shm_open(SharedMemName, O_CREAT | O_RDWR, 0640);

    if (fileDescriptor == -1) {
        return OSMP_ERROR;
    }

    int ftrunc = ftruncate(fileDescriptor, SharedMemSize);

    if (ftrunc == -1) {
        printf("Fehler bei ftruncate %s\n", strerror(errno));
        return OSMP_ERROR;
    }


    void *map = mmap(NULL, SharedMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);

    if (map == MAP_FAILED) {
        printf("Mapping Fail: %s\n", strerror(errno));
        shm_unlink(SharedMemName);
        return OSMP_ERROR;
    }

    return OSMP_SUCCESS;
}

int OSMP_Finalize() {
    printf("finalize\n");
    return 0;
}

int OSMP_Size(int *size) {
    *size = counter;
    return OSMP_SUCCESS;
}

int OSMP_Rank(int *rank) {
    pid_t pid = getpid();
    *rank = pid;
    return OSMP_SUCCESS;
}

int OSMP_Send() {
    printf("send\n");
    return 0;
}

int OSMP_Recv() {
    printf("receive\n");
    return 0;
}

int OSMP_Bcast() {
    printf("broadcast\n");
    return 0;
}