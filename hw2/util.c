#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>



void init_server(){
    char server_dir[20] = "./server_dir";
    char client_dir[20] = "./client_dir";
    char banlist[20] = "./server_dir/banlist";
    char cmd[50];
    struct stat sb;

    if (!(stat(server_dir, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        sprintf(cmd, "mkdir %s", server_dir);
        system(cmd);
    }
    if (!(access(banlist, F_OK) == 0)){
        sprintf(cmd, "touch %s", banlist);
        system(cmd);
    }
    if (!(stat(client_dir, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        sprintf(cmd, "mkdir %s", client_dir);
        system(cmd);
    }
}

void init_user(char* user){
    char cmd[50];
    char dir[20];
    struct stat sb;

    sprintf(dir, "./server_dir/%s", user);
    if (!(stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        sprintf(cmd, "mkdir %s", dir);
        system(cmd);
    }
}

int check_ban(char* user){
    int exist = 0;
    FILE* f;
    char* line = "";
    size_t len = 0;
    f = fopen("./server_dir/banlist", "r");
    if (f == NULL)
        ERR_EXIT("banlist open error");

    while ((getline(&line, &len, f)) != -1) {
        strtok(line, "\n");
        if (strcmp(user, line) == 0) {
            exist = 1;
            break;
        }
    }

    return exist;
}

void send_file(FILE *fp, int sockfd){
    int n;
    char data[1024] = {0};
 
    while(fgets(data, 1024, fp) != NULL) {
        printf("%s\n", data);
        if (send(sockfd, data, strlen(data), 0) == -1) {
            ERR_EXIT("[-]Error in sending file.");
        }
        memset(data, '\0', sizeof(char) * 1024);

        read(sockfd, data, sizeof(data) - 1);
        memset(data, '\0', sizeof(char) * 1024);
        
    }

    if (send(sockfd, "Finish", strlen("Finish"), 0) == -1) {
        ERR_EXIT("[-]Error in sending file.");
    }
}

void write_file(int sockfd, char* filename){
    int n;
    FILE *fp;
    char buffer[1024] = {0};

    fp = fopen(filename, "wb");
    while (1) {
        n = recv(sockfd, buffer, 1024, 0);

        if (strcmp(buffer, "Finish") == 0){
            break;
        }
        fprintf(fp, "%s", buffer);
        memset(buffer, '\0', sizeof(char) * 1024);
        
        send(sockfd, "OK", strlen("OK"), 0);
    }
    fclose(fp);
    return;
}




