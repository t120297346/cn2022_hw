#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <dirent.h>

#include "util.h"

#define BUFF_SIZE 1024
#define PORT 8787
#define ERR_EXIT(a){ perror(a); exit(1); }

int main(int argc, char *argv[]){
    int opt = 1; 
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 15, activity, read_bytes, sd;
    int max_sd;

    struct sockaddr_in address;
    char buffer[BUFF_SIZE] = {};

    // set of socket descriptors
    fd_set readfds;

    // message
    char message[BUFF_SIZE] = "Hello";

    for (int i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }

    // create a master socket 
    if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0){
        ERR_EXIT("socket failed\n")
    }

    //set master socket to allow multiple connections ,
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        ERR_EXIT("setsockopt");
    }

    // Set server address information
    bzero(&address, sizeof(address)); // erase the data
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if(argc > 1) {
        address.sin_port = htons((uint16_t)strtol(argv[1], NULL, 10));
    }
    else {
        address.sin_port = htons(PORT);
    }
    
    // Bind the server file descriptor to the server address
    if(bind(master_socket, (struct sockaddr *)&address , sizeof(address)) < 0) {
        ERR_EXIT("bind failed\n");
    }
        
    // Listen on the server file descriptor
    if(listen(master_socket , 3) < 0) {
        ERR_EXIT("listen failed\n");
    }

    addrlen = sizeof(address);
    init_server();
    //printf("Waiting for connections ...\n");
    
    while(1) {
	//clear the socket set 
        FD_ZERO(&readfds);
	
	//add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

	//add child sockets to set
        for(int i = 0; i < max_clients; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET(sd, &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

	    activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
	    if((activity < 0) && (errno!=EINTR)) {
            printf("select error\n");
        }

	    //If something happened on the master socket ,
        //then its an incoming connection
        if(FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                ERR_EXIT("accept failed");
            }

            if((read_bytes = read(new_socket, buffer, sizeof(buffer) - 1)) < 0){
                ERR_EXIT("receive failed\n");
            }

            if(check_ban(buffer) == 0){
                printf("Accept a new connection on socket [%d]. Login as %s\n", new_socket, buffer);
                init_user(buffer);

                //send new connection greeting message
                if(send(new_socket, "Hello", strlen("Hello"), 0) != strlen("Hello")) {
                    ERR_EXIT("begin send error");
                }
                

                //add new socket to array of sockets
                for(int i = 0; i < max_clients; i++) {
                    //if position is empty
                    if( client_socket[i] == 0 ){
                        client_socket[i] = new_socket;
                        //printf("Adding to list of sockets as %d\n" , i);
                        break;
                    }
                }
            }
            else {
                if(send(new_socket, "Ban", strlen("Ban"), 0) != strlen("Ban")) {
                    ERR_EXIT("begin send error");
                }
            }
        }

	    //else its some IO operation on some other socket
        for(int i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if(FD_ISSET(sd, &readfds)) {
                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                //Check if it was for closing , and also read the
                //incoming message
                if((read_bytes = read(sd, buffer, sizeof(buffer))) == 0) {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    //printf("Host disconnected , ip %s , port %d \n",
                    //      inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                else {
                    /* Processing socket buffer*/
                    char* temp = strdup(buffer);
                    char* save = temp;
                    char* user = strsep(&temp, " ");
                    char* cmd = strsep(&temp, " ");

                    /* Basic operation */
                    if (strcmp(cmd, "ls") == 0) {
                        char files[1024] = {};
                        DIR *d;
                        struct dirent *dir;
                        char path[30];

                        sprintf(path, "./server_dir/%s", user);
                        d = opendir(path);
                        if (d) {
                            while ((dir = readdir(d)) != NULL) {
                                if(!(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0))
                                    sprintf(files, "%s%s\n", files, dir->d_name);
                            }
                            closedir(d);
                        }
                        if(strlen(files) == 0){
                            sprintf(files, "%s\n", files);
                        }

                        memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                        if(send(new_socket, files, strlen(files), 0) != strlen(files)) {
                            ERR_EXIT("ls errer");
                        }
                    }
                    else if (strcmp(cmd, "put") == 0) {
                        char* filename = strsep(&temp, " ");
                        pid_t pid;

                        memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                        sprintf(buffer, "OK");
                        if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                            ERR_EXIT("put init errer");
                        }

                        char path[50];
                        sprintf(path, "./server_dir/%s/%s", user, filename);
                        write_file(new_socket, path);
                        
                        memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                        sprintf(buffer, "Done");
                        if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                            ERR_EXIT("put finish errer");
                        }
                    }

                    /* admin operation */
                    else if (strcmp(cmd, "banlist") == 0) {
                        if (strcmp(user, "admin") == 0){
                            char banlist[1024] = {};
                            FILE* f;
                            char* line = "";
                            size_t len = 0;
                            
                            f = fopen("./server_dir/banlist", "r");
                            if (f == NULL)
                                ERR_EXIT("banlist open error");

                            while ((getline(&line, &len, f)) != -1) {
                                sprintf(banlist, "%s%s", banlist, line);
                            } 

                            if (strcmp(banlist, "") == 0)
                                sprintf(banlist, "\n");
                            if(send(new_socket, banlist, strlen(banlist), 0) != strlen(banlist)) {
                                ERR_EXIT("banlist send errer");
                            }
                        }
                        else{
                            char* message = "Permission denied\n";
                            if(send(new_socket, message, strlen(message), 0) != strlen(message)) {
                                ERR_EXIT("banlist send errer");
                            }
                        }
                    }
                    else if (strcmp(cmd, "ban") == 0) {
                        if (strcmp(user, "admin") == 0){
                            char* ban_user = strsep(&temp, " ");
                            if (strcmp(ban_user, "admin") == 0){
                                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                                sprintf(buffer, "You cannot ban yourself!\n");
                                if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                                    ERR_EXIT("banlist send errer");
                                }
                            }
                            else{
                                int exist = 0;
                                FILE* f;
                                char* line = "";
                                size_t len = 0;

                                f = fopen("./server_dir/banlist", "r+");
                                if (f == NULL)
                                    ERR_EXIT("banlist open error");
                                
                                while ((getline(&line, &len, f)) != -1) {
                                    strtok(line, "\n");
                                    if (strcmp(ban_user, line) == 0) {
                                        exist = 1;
                                        break;
                                    }
                                }

                                if (exist){
                                    memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                                    sprintf(buffer, "User %s is already on the blocklist!\n", ban_user);
                                    if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                                        ERR_EXIT("banlist send errer");
                                    }
                                }
                                else{
                                    fseek(f, 0, SEEK_END);
                                    fprintf(f, "%s\n", ban_user);

                                    memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                                    sprintf(buffer, "Ban %s successfully!\n", ban_user);
                                    if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                                        ERR_EXIT("banlist send errer");
                                    }                       
                                }
                                fclose(f);
                            }
                        }
                        else{
                            char* message = "Permission denied\n";
                            if(send(new_socket, message, strlen(message), 0) != strlen(message)) {
                                ERR_EXIT("banlist send errer");
                            }
                        }
                    }
                    else if(strcmp(cmd, "unban") == 0){
                        if (strcmp(user, "admin") == 0) {
                            char* ban_user = strsep(&temp, " ");
                            int exist = 0;
                            FILE* f;
                            char* line = "";
                            size_t len = 0;
                            char new_file[BUFF_SIZE] = {};

                            f = fopen("./server_dir/banlist", "r+");
                            if (f == NULL)
                                ERR_EXIT("banlist open error");

                            while ((getline(&line, &len, f)) != -1) {
                                strtok(line, "\n");
                                if (strcmp(ban_user, line) == 0) {
                                    exist = 1;
                                }
                                else {
                                    sprintf(new_file, "%s%s\n", new_file, line);
                                }
                            }
                            fclose(f);

                            if (exist){
                                f = fopen("./server_dir/banlist", "w");
                                fprintf(f, "%s", new_file);
                                fclose(f);

                                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                                sprintf(buffer, "Successfully removed %s from the blocklist!\n", ban_user);
                                if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                                    ERR_EXIT("banlist send errer");
                                }
                            }
                            else{
                                memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
                                sprintf(buffer, "User %s is not on blocklist!\n", ban_user);
                                if(send(new_socket, buffer, strlen(buffer), 0) != strlen(buffer)) {
                                    ERR_EXIT("banlist send errer");
                                }
                            }
                        }
                        else {
                            char* message = "Permission denied\n";
                            if(send(new_socket, message, strlen(message), 0) != strlen(message)) {
                                ERR_EXIT("banlist send errer");
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
