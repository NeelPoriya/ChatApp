#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "user.c"

#define PORT 8080

int clientSocket, ret;
struct sockaddr_in serverAddr;
char buffer[1024];

void connectToServer()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");
}

void receiveFile()
{
    while (recv(clientSocket, buffer, sizeof(buffer), 0))
    {
        if (!strcmp(buffer, ":finish\n"))
            break;
        printf("%s", buffer);
    }
}

void refresh()
{
    system("clear");
    receiveFile();

    printf("Enter your message : ");
}

int main()
{
    connectToServer();
    char *username = malloc(sizeof(char) * 20);
    int passed = 0;

    // Ask for creating a user or login
    printf("------------------------- Welcome to ChatApp ------------------------------\n");
    while (!passed)
    {
        printf("1. Create a new User account.\n");
        printf("2. Login\n3. Quit\nEnter your choice : ");
        int choice;
        scanf("%d", &choice), getchar();

        switch (choice)
        {
        case 1:;
            struct User *newUser = createUser();
            if (newUser == NULL)
                break;
            else
            {
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ":create@");
                strcat(buffer, convertToString(newUser));
                printf("Sending : %s\n", buffer);
                send(clientSocket, buffer, sizeof(buffer), 0);

                bzero(buffer, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);

                printf("Server response : %s\n", buffer);
                if (!strcmp(buffer, "New User added successfully."))
                {
                    passed = 1;
                }
            }
            break;

        case 2:;
            char *password = malloc(sizeof(char) * 30);

            printf("Enter your username : ");
            scanf("%[^\n]s", username), getchar();

            printf("Enter your password : ");
            scanf("%[^\n]s", password), getchar();

            bzero(buffer, sizeof(buffer));
            strcat(buffer, ":check@");
            strcat(buffer, username);
            strcat(buffer, ";");
            strcat(buffer, password);
            send(clientSocket, buffer, sizeof(buffer), 0);

            bzero(buffer, sizeof(buffer));
            recv(clientSocket, buffer, sizeof(buffer), 0);
            printf("Server response : %s\n", buffer);

            if (!strcmp(buffer, "Login Successful."))
            {
                passed = 1;
            }
            break;
        case 3:
            close(clientSocket);
            exit(0);
            break;
        default:
            break;
        }
    }

    char *room_name = (char *)malloc(sizeof(char) * 20);
    printf("------------------ Welcome to room selection ------------------\n");
    printf("(If you don't have room, just type a name and you will create one)\nEnter your room name : ");
    scanf("%[^\n]s", room_name);

    bzero(buffer, sizeof(buffer));
    strcat(buffer, ":room@");
    strcat(buffer, room_name);
    send(clientSocket, buffer, sizeof(buffer), 0);

    // if (fork() == 0)
    // {
    //     while (1)
    //     {
    //         sleep(2);
    //         char ref[50] = ":refresh@";
    //         strcat(ref, room_name);

    //         send(clientSocket, ref, sizeof(ref), 0);
    //         refresh();
    //         fflush(stdin);
    //     }
    // }
    // else
    {
        while (1)
        {
            system("clear");
            receiveFile();

            printf("Enter your message : ");
            char *message = (char *)malloc(sizeof(char) * 100);
            getchar();
            scanf("%[^\n]s", message);

            // printf("%s\n", message);
            char *cmd = strtok(message, "@");
            // printf("%s\n", cmd);

            if (!strcmp(cmd, "load"))
            {
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ":load@");
                strcat(buffer, room_name);
                send(clientSocket, buffer, sizeof(buffer), 0);
            }

            else if (!strcmp(cmd, "profile"))
            {
                // user is asking for profile
                char *other_username = strtok(NULL, "@");
                bzero(buffer, sizeof(buffer));

                strcpy(buffer, ":profile@");
                strcat(buffer, other_username);

                send(clientSocket, buffer, sizeof(buffer), 0);
            }

            else
            {
                // send the message
                // format -> :message@complete_message;room_name;user_name
                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ":message@");
                strcat(buffer, message);
                strcat(buffer, ";");
                strcat(buffer, room_name);
                strcat(buffer, ";");
                strcat(buffer, username);
                send(clientSocket, buffer, sizeof(buffer), 0);
            }
        }
    }
    close(clientSocket);
    return 0;
}
