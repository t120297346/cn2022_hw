#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//server
void init_server();
void init_user(char* user);

void send_file(FILE *fp, int sockfd);
void write_file(int sockfd);
