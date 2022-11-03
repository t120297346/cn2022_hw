#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ERR_EXIT(a){ perror(a); exit(1); }

//server
void init_server();
void init_user(char* user);
int check_ban(char* user);

void send_file(FILE *fp, int sockfd);
void write_file(int sockfd, char* filename);
