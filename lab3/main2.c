#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/wait.h>

#define QUEUE_NAME "/queue-name"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define FIRST_PROGRAM_NAME "main1"

int main(int argc, char *argv[]) {
    mqd_t queue;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((queue = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Failed to open queue");
        return -1;
    }

    pid_t childPid1;
    pid_t childPid2;

    childPid1 = fork();
    if (childPid1 == -1) {
        perror("Failed to fork first process");
        return -1;
    }

    if (childPid1 == 0) {
        (void)execl(FIRST_PROGRAM_NAME, FIRST_PROGRAM_NAME, argv[1], NULL);
        exit(0);
    } else { 
        childPid2 = fork();

        if (childPid2 == -1) {
            perror("Failed to fork second process");
            return -1;
        }

        if (childPid2 == 0) { 
            (void)execl(FIRST_PROGRAM_NAME, FIRST_PROGRAM_NAME, argv[2], NULL);
            exit(0);
        } else { 
            waitpid(childPid1, NULL, 0);
            waitpid(childPid2, NULL, 0);
        }
    }

    char buffer1[MAX_MSG_SIZE] = "";
    char buffer2[MAX_MSG_SIZE] = "";

    if (mq_receive(queue, buffer1, MAX_MSG_SIZE, NULL) == -1) {
        perror("Failed to receive a message");
        return -1;
    }

     if (mq_receive(queue, buffer2, MAX_MSG_SIZE, NULL) == -1) {
        perror("Failed to receive a message");
        return -1;
    }

    char outputBuffer[BUFSIZ + 1];
    char *outputFileName;
    outputFileName = argv[3];
    
    FILE *output = fopen(outputFileName, "w+");

    int i = 0;
    for (i = 0; i < MAX_MSG_SIZE; i++) {
        if (strlen(buffer1) < i) {
            buffer1[i] = 0;
        }
        if (strlen(buffer2) < i) {
            buffer2[i] = 0;
        }

        if (buffer1[i] == buffer2[i]) {
            if (buffer1[i] == 0) {
                break;
            }

            outputBuffer[i] = buffer1[i];
        }
        else {
            outputBuffer[i] = buffer1[i] ^ buffer2[i];
        }

        fprintf(output, "%c", outputBuffer[i]);
    }

    fclose(output);
    mq_close(queue);
    return 0;
}