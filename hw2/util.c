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

void send_file(FILE *fp, int sockfd){
    int n;
    char data[1024] = {0};
 
    while(fgets(data, 1024, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(data, 1024);
    }
}

void write_file(int sockfd){
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[1024];

    fp = fopen(filename, "w");
    while (1) {
        n = recv(sockfd, buffer, 1024, 0);
        if (n <= 0){
            break;
            return;
        }
        fprintf(fp, "%s", buffer);
        bzero(buffer, 1024);
    }
    return;
}




