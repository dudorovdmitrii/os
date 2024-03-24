#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define QUEUE_NAME "/queue-name"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256

int main (int argc, char **argv)
{
    char *fileName;
    fileName = argv[1];
    FILE *file = fopen(fileName, "r");
    if (file == NULL) { 
        printf("Failed to open file %s \n", fileName);
        return -1;
    }

    char buff[MAX_MSG_SIZE];
    fgets(buff, sizeof(buff), file);
    fclose(file);

    mqd_t queue;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((queue = mq_open(QUEUE_NAME, O_WRONLY, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Failed to open queue");
        return -1;
    }

    if (mq_send (queue, buff, MAX_MSG_SIZE, 0) == -1) {
        perror("Failed to send a message");
        return -1;
    }

    return 0;
}