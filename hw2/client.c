#include<sys/socket.h> 
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<unistd.h> 
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#define BUFF_SIZE 1024
#define PORT 8787
#define ERR_EXIT(a){ perror(a); exit(1); }

int main(int argc , char *argv[]){
    int sockfd, read_byte;
    struct sockaddr_in addr;
    char buffer[BUFF_SIZE] = {};

    // Get socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        ERR_EXIT("socket failed\n");
    }

    // Set server address
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    if(argc > 1){
        addr.sin_addr.s_addr = inet_addr(strsep(&argv[2], ":"));
        addr.sin_port = htons((uint16_t)strtol(argv[2], NULL, 10));
    }
    else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(PORT);
    }

    // Connect to the server
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        ERR_EXIT("connect failed\n");
    }

    if(send(sockfd, argv[1], strlen(argv[1]), 0) != strlen(argv[1])) {
        ERR_EXIT("send failed");
    }
    // Receive message from server
    if((read_byte = read(sockfd, buffer, sizeof(buffer) - 1)) < 0){
        ERR_EXIT("receive failed\n");
    }
    if(strcmp(buffer, "Hello") == 0){
        printf("User %s successfully logged in\n", argv[1]);
    }
    
    while(1){
        printf("$ ");

        /* Processing input arguments */
        char* line = NULL;
        size_t len = 0;
        getline(&line, &len, stdin);
        char* temp = strdup(line);
        char* save = temp;
        strsep(&temp, "\n");
        char* cmd = strtok(save, " ");

        /* basic operations */
        if (strcmp(cmd, "ls") == 0){
            memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
            sprintf(buffer, "%s %s ", argv[1], cmd);
            if(send(sockfd, buffer, strlen(buffer), 0) != strlen(buffer)) {
                ERR_EXIT("send ls failed");
            }

            memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
            if((read_byte = read(sockfd, buffer, sizeof(buffer) - 1)) < 0){
                ERR_EXIT("receive ls failed\n");
            }
            printf("%s", buffer);

        }
        else if(strcmp(cmd, "put") == 0){
            char filename[30];
            scanf("%s",filename);
            printf("%s\n", filename);
        }
        else if (strcmp(cmd, "exit") == 0){
            break;
        }
        /* admin operations */
        else if (strcmp(cmd, "banlist") == 0) {
            sprintf(buffer, "%s %s ", argv[1], cmd);
            if(send(sockfd, buffer, strlen(buffer), 0) != strlen(buffer)) {
                ERR_EXIT("send banlist failed");
            }

            memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
            if((read_byte = read(sockfd, buffer, sizeof(buffer) - 1)) < 0){
                ERR_EXIT("receive banlist failed\n");
            }
            printf("%s", buffer);
        }
        else if (strcmp(cmd, "ban") == 0) {
            char* ban_users = strdup(cmd);
            ban_users = strtok(NULL, " ");
            while (ban_users != NULL) {
                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                sprintf(buffer, "%s %s %s ", argv[1], cmd, ban_users);
                if(send(sockfd, buffer, strlen(buffer), 0) != strlen(buffer)) {
                    ERR_EXIT("send ban failed");
                }
                
                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                if((read_byte = read(sockfd, buffer, sizeof(buffer) - 1)) < 0){
                    ERR_EXIT("receive ban failed\n");
                }

                printf("%s", buffer);
                if(strcmp(buffer, "Permission denied\n") == 0)
                    break;

                ban_users = strtok(NULL, " ");
            }
        }
        else if (strcmp(cmd, "unban") == 0) {
            char* ban_users = strdup(cmd);
            ban_users = strtok(NULL, " ");
            while (ban_users != NULL) {
                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                sprintf(buffer, "%s %s %s ", argv[1], cmd, ban_users);
                if(send(sockfd, buffer, strlen(buffer), 0) != strlen(buffer)) {
                    ERR_EXIT("send unban failed");
                }

                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                if((read_byte = read(sockfd, buffer, sizeof(buffer) - 1)) < 0){
                    ERR_EXIT("receive unban failed\n");
                }

                printf("%s", buffer);
                if(strcmp(buffer, "Permission denied\n") == 0)
                    break;

                ban_users = strtok(NULL, " ");
            }
        }
        /* undefined command */
        else {
            printf("Command Not Found!\n");
        }
    }
    

    close(sockfd);
    return 0;
}

