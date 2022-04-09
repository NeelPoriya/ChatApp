#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "user.c"

#define PORT 8080
char buffer[1024];

void send_file(char *path, int sockfd)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        printf("file not found!");
        return;
    }

    while (fgets(buffer, sizeof(buffer), file))
    {
        send(sockfd, buffer, sizeof(buffer), 0);
        bzero(buffer, sizeof(buffer));
    }
    if (send(sockfd, ":finish\n", 9, 0) == -1)
    {
        printf("error sending stuff.\n");
    }
    fclose(file);
}

int main()
{

    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    pid_t childpid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int tr = 1;
    // kill "Address already in use" error message
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-]Error in binding.\n");

        perror(" Some error");
        close(sockfd);
        exit(1);
    }
    printf("[+]Bind to port %d\n", PORT);

    if (listen(sockfd, 10) == 0)
    {
        printf("[+]Listening....\n");
    }
    else
    {
        printf("[-]Error in binding.\n");
        perror(" Some error");
        close(sockfd);
    }

    while (1)
    {
        int newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newSocket < 0)
        {
            printf("Error in socket connection\n");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        if ((childpid = fork()) == 0)
        {
            close(sockfd);

            while (1)
            {
                bzero(buffer, sizeof(buffer));
                if (recv(newSocket, buffer, 1024, 0) <= 0)
                {
                    printf("Connection broken!\n");
                    exit(1);
                }
                if (strlen(buffer) == 0)
                {
                    continue;
                }

                char *command = strtok(buffer, "@");
                if (strcmp(command, ":exit") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }
                else if (strcmp(command, ":create") == 0)
                {
                    char *user_string = strtok(NULL, "@");
                    saveUser(user_string);
                    bzero(buffer, sizeof(buffer));
                    strcpy(buffer, "New User added successfully.");
                    send(newSocket, buffer, sizeof(buffer), 0);
                }
                else if (strcmp(command, ":check") == 0)
                {
                    char *usr = strtok(NULL, ";");
                    char *passwd = strtok(NULL, ";");

                    char username[20], password[30];
                    strcpy(username, usr);
                    strcpy(password, passwd);
                    bzero(buffer, sizeof(buffer));
                    if (checkUser(username, password))
                    {
                        strcpy(buffer, "Login Successful.");
                    }
                    else
                    {
                        strcpy(buffer, "No such user exist.");
                    }
                    if (send(newSocket, buffer, sizeof(buffer), 0) == -1)
                    {
                        perror("Error sending in data");
                        exit(EXIT_FAILURE);
                        bzero(buffer, sizeof(buffer));
                    };
                }
                else if (strcmp(command, ":room") == 0)
                {
                    char *room_name_recv = strtok(NULL, ";");

                    char path[40];
                    strcpy(path, "./rooms/");
                    strcat(path, room_name_recv);
                    strcat(path, ".txt");
                    // printf("path : %s\n", path);

                    send_file(path, newSocket);

                    // if file already exists, send all data to client
                }
                else if (strcmp(command, ":message") == 0)
                {
                    char *message = strtok(NULL, ";");
                    char *room_name = strtok(NULL, ";");
                    char *username = strtok(NULL, ";");
                    // printf("Adding : %s to Room %s by user %s\n", message, room_name, username);

                    char path[30] = "./rooms/";
                    strcat(path, room_name);
                    strcat(path, ".txt");
                    FILE *fp = fopen(path, "a");

                    char formatted_message[strlen(message) + strlen(username) + 5];
                    strcpy(formatted_message, username);
                    strcat(formatted_message, ": ");
                    strcat(formatted_message, message);
                    strcat(formatted_message, "\n");

                    fprintf(fp, formatted_message, sizeof(formatted_message));
                    fclose(fp);

                    send_file(path, newSocket);
                }
                // else if (strcmp(command, ":refresh") == 0)
                // {
                //     printf("Request for file\n");
                //     char *file_name = strtok(NULL, ";");
                //     char path[50] = "./rooms/";
                //     strcat(path, file_name);
                //     strcat(path, ".txt");

                //     printf("path : %s\n", path);
                //     send_file(path, newSocket);
                // }
                else if (strcmp(command, ":load") == 0)
                {
                    char path[50] = "./rooms/";
                    char *file_name = strtok(NULL, ";");
                    strcat(path, file_name);
                    strcat(path, ".txt");
                    printf("path : %s\n", path);

                    send_file(path, newSocket);
                }
                else if (strcmp(command, ":profile") == 0)
                {
                    char *username_received = strtok(NULL, " ");
                    char username[30];
                    strcpy(username, username_received);
                    printf("Username received : %s\n", username_received);
                    char *profile = getUserProfileString(getUser(username_received));
                    bzero(buffer, sizeof(buffer));

                    strcpy(buffer, profile);
                    send(newSocket, buffer, sizeof(buffer), 0);
                    send(newSocket, ":finish\n", 10, 0);
                }
                else
                {
                    printf("%s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }
            }
            close(newSocket);
            exit(0);
        }
        close(newSocket);
    }

    printf("I am ending\n");
    return 0;
}
