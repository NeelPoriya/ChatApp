#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "user.c"

#define PORT 8080
#define COLOURS 12

int clientSocket, ret;
struct sockaddr_in serverAddr;
char buffer[1024];

char *temp;
char *room_name, *username;
char **usernames;
char *colours[] = { "\033[3;91m","\033[3;92m","\033[3;93m","\033[3;94m","\033[3;95m","\033[3;96m","\033[3;33m","\033[3;33m", "\033[3;35m", "\033[3;31m", "\033[3;32m", "\033[3;36m"};

int isReading = 0;

void connectToServer()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, temp, &(serverAddr.sin_addr));

    ret = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
}

void exit_handler()
{
    send(clientSocket, ":exit", 6, 0);
    close(clientSocket);
    exit(0);
}

void *send_message_func(void *arg)
{
    char *message = (char *)malloc(sizeof(char) * 100);
    while (1)
    {
        getchar();
        do{
            scanf("%[^\n]s", message);
        }while(strlen(message) == 0);

        if (strlen(message) == 0){
            printf("\033[0;32m");
            printf("\nEnter your message : \033[0m \n");
            continue;
        }
        fflush(stdin);

        char *cmd = strtok(message, "@");

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
        else if (!strcmp(cmd, "exit"))
        {
            exit_handler();
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

int get_colour_index(char *username)
{
    for (int i = 0; i < 100; ++i)
    {
        if (usernames[i] == NULL)
        {
            usernames[i] = (char *)malloc(sizeof(char) * 30);
            strcpy(usernames[i], username);
            return i % COLOURS;
        }
        if ((usernames[i] != NULL) && (strcmp(username, usernames[i]) == 0))
        {
            return i % COLOURS;
        }
    }
}

int first_time_receiving = 1;

void *receive_message_func(void *arg)
{
    bzero(buffer, sizeof(buffer));
    while (recv(clientSocket, buffer, sizeof(buffer), 0))
    {
        if (!strcmp(buffer, ":clear\n"))
        {
            // for (int i = 0; i < 40; ++i)
            //     printf("\n");
            system("clear");
            // sleep(0.5);
        }
        else if (!strcmp(buffer, ":finish\n"))
        {
            bzero(buffer, sizeof(buffer));
            printf("\033[0;32m");
            printf("\nEnter your message : \033[0m \n");
            first_time_receiving = 1;
        }
        else
        {
            if(first_time_receiving){
                first_time_receiving = 0;
                // for(int i = 0; i < 40; ++i) printf("\n");
                system("clear");
            }
            
            // printf("%s", buffer);
            char* profile_test = strtok(buffer, "\n");
            if(!strcmp(profile_test, ":profile")){
                while(profile_test!= NULL){
                    profile_test = strtok(NULL, "\n");
                    if(!strcmp(profile_test, ":end")){
                        break;
                    }
                    printf("%s\n", profile_test);
                }
                bzero(buffer, sizeof(buffer));
                continue;
            }
            char* usrname = strtok(buffer, ":");
            char* chat = strtok(NULL, "\n");

            int colour_index = get_colour_index(usrname);
            printf("%s", colours[colour_index]);
            printf("%s", usrname);

            printf("\033[0;37m:%s\n", chat);

        }

        bzero(buffer, sizeof(buffer));
    }
}

int main(int argc, char *argv[])
{

    usernames = (char **)malloc(sizeof(char *) * 100);
    memset(usernames, 0, sizeof(usernames));

    signal(SIGINT, exit_handler);

    //temp = argv[1];
    connectToServer();
    username = (char *)malloc(sizeof(char) * 30);
    room_name = (char *)malloc(sizeof(char) * 20);
    int passed = 0;

    // Ask for creating a user or login
    
    FILE* fp = fopen("chatapp.txt", "r");
    char chatapp_text[1000];
    while(fgets(chatapp_text, sizeof(chatapp_text), fp)){
        printf("%s", chatapp_text);
    }
    printf("\n\n");
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
                sprintf(username, "%s", newUser->username);

                bzero(buffer, sizeof(buffer));
                strcpy(buffer, ":create@");
                strcat(buffer, convertToString(newUser));

                // printf("Sending : %s\n", buffer);
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
            char *password = malloc(sizeof(char) * 20);

            do{
                printf("Enter your username : ");
                scanf("%[^\n]s", username), getchar();
            }while(username == NULL || strlen(username) == 0 || strlen(username) > 30);

            do{
                printf("Enter your password : ");
                scanf("%[^\n]s", password), getchar();
            }while(password == NULL || strlen(password) == 0 || strlen(password) > 20);

            bzero(buffer, sizeof(buffer));
            strcat(buffer, ":check@");
            strcat(buffer, username);
            strcat(buffer, ";");
            strcat(buffer, password);
            send(clientSocket, buffer, sizeof(buffer), 0);

            bzero(buffer, sizeof(buffer));
            recv(clientSocket, buffer, sizeof(buffer), 0);
            printf("\nServer response : %s\n", buffer);

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

    printf("\n\n------------------ Welcome to room selection ------------------\n");
    printf("(If you don't have room, just type a name and you will create one)\n\nEnter your room name : ");
    scanf("%[^\n]s", room_name);

    bzero(buffer, sizeof(buffer));
    strcat(buffer, ":room@");
    strcat(buffer, room_name);
    send(clientSocket, buffer, sizeof(buffer), 0);

    system("clear");

    pthread_t send_message_thread;
    pthread_create(&send_message_thread, NULL, send_message_func, NULL);

    pthread_t receive_message_thread;
    pthread_create(&receive_message_thread, NULL, receive_message_func, NULL);

    while (1)
    {
        ;
    }
    close(clientSocket);
    return 0;
}
