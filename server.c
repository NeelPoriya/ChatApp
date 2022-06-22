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
#include <sys/signal.h>
#include <pthread.h>
#include "user.c"

#define PORT 8080
#define MAX_CLIENTS 500
char buffer[1024];

struct client
{
    struct User user;
    struct sockaddr_in address;
    char username[20];
    char room_name[20];
    int sockfd;
};

struct client *clients[MAX_CLIENTS];
int no_of_clients = 0;

// returns 1 for failuer
// and 0 for success
int addClient(struct client *cli)
{
    if (no_of_clients > MAX_CLIENTS || cli == NULL)
        return 1;
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i] == NULL)
        {
            clients[i] = cli;
            break;
        }
    }
    return 0;
}

// returns 1 for failuer
// and 0 for success
int removeClient(struct client *cli)
{
    if (no_of_clients <= 0 || cli == NULL)
        return 1;
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i] == cli)
        {
            free(clients[i]);
            clients[i] = NULL;
            break;
        }
    }
    return 0;
}

void send_file(char *path, int sockfd, int clearPageFlag)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        printf("file not found!");
        return; 
    }

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
        {
            printf("Error send to someone\n");
            break;
        }
        bzero(buffer, sizeof(buffer));
    }
    if (send(sockfd, ":finish\n", 9, 0) == -1)
    {
        printf("Error sending stuff.\n");
    }
    fclose(file);
}

void send_to_all_members(char *room, char *path)
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i] != NULL)
        {
            if (!strcmp(clients[i]->room_name, room))
            {
                send_file(path, clients[i]->sockfd, 1);
            }
        }
    }
}

void *handle_client(void *arg)
{
    struct client *cli = (struct client *)arg;
    int newSocket = cli->sockfd;

    if (addClient(cli))
    {
        printf("Unable to add client to main array\n");
        pthread_exit(NULL);
    };
    no_of_clients++;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        if (recv(newSocket, buffer, 1024, 0) <= 0)
        {
            printf("Connection with %s ended!\n", cli->username);
            pthread_exit(NULL);
        }
        if (strlen(buffer) == 0)
        {
            continue;
        }

        char *command = strtok(buffer, "@");
        if (strcmp(command, ":exit") == 0)
        {
            printf("Disconnected from %s:%d\n", inet_ntoa(cli->address.sin_addr), ntohs(cli->address.sin_port));
            break;
        }
        else if (strcmp(command, ":create") == 0)
        {
            char *user_string = strtok(NULL, "@");
            saveUser(user_string);

            char temp[50];
            sprintf(temp, "%s", user_string);
            strtok(temp, ";");
            char *user_name = strtok(NULL, ";");

            strcpy(cli->username, user_name);

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
            strcpy(cli->username, usr);
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
            }

            updateSession(cli->username, 0);
        }
        else if (strcmp(command, ":room") == 0)
        {
            char *room_name_recv = strtok(NULL, ";");
            strcpy(cli->room_name, room_name_recv);

            char path[40];
            strcpy(path, "./rooms/");
            strcat(path, room_name_recv);
            strcat(path, ".txt");

            if (access(path, 0) != 0)
            {
                FILE *fp = fopen(path, "w");
                fclose(fp);
            }
            send_file(path, newSocket, 0);

            // if file already exists, send all data to client
        }
        else if (strcmp(command, ":message") == 0)
        {
            char *message = strtok(NULL, ";");
            char *room_name = strtok(NULL, ";");
            char *username = strtok(NULL, ";");

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

            send_to_all_members(cli->room_name, path);
        }
        else if (strcmp(command, ":load") == 0)
        {
            char path[50] = "./rooms/";
            char *file_name = strtok(NULL, ";");
            strcat(path, file_name);
            strcat(path, ".txt");
            // printf("path : %s\n", path);

            send_file(path, newSocket, 1);
        }
        else if (strcmp(command, ":profile") == 0)
        {
            char *username_received = strtok(NULL, " ");
            char username[30];
            strcpy(username, username_received);
            
            char *profile = getUserProfileString(getUser(username_received));
            bzero(buffer, sizeof(buffer));
            
            strcpy(buffer, ":profile\n");
            strcat(buffer, profile);
            strcat(buffer, ":end\n");
            send(newSocket, buffer, sizeof(buffer), 0);
            send(newSocket, ":finish\n", 10, 0);
        }
        else
        {
            send(newSocket, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof(buffer));
        }
    }

    if (removeClient(cli))
    {
        printf("Unable to remove client from main array.\n");
        pthread_exit(NULL);
    };
    no_of_clients--;

    updateSession(cli->username, 1);
    close(newSocket);
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
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
    printf("[+]Connection Established\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; //localhost

    // inet_pton(AF_INET, argv[1], &(serverAddr.sin_addr));

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

        perror("");
        close(sockfd);
        exit(1);
    }
    printf("[+]Binding successful!\n");

    if (listen(sockfd, 100) != 0)
    {
        printf("[-]Error in binding.\n");
        perror("");
        close(sockfd);
    }
    printf("[+]Listening\n");

    while (1)
    {
        int newClientFd = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newClientFd < 0)
        {
            printf("Error in socket connection\n");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        pthread_t pt;
        struct client *c = (struct client *)malloc(sizeof(struct client));

        c->sockfd = newClientFd;
        c->address = newAddr;
        pthread_create(&pt, NULL, handle_client, (void *)c);
    }
    return 0;
}
