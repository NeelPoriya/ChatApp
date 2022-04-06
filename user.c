#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

struct User
{
    char *name;
    char *username;
    char *password;
    int age;
    char *last_login;
    char *status;
};

int checkUniqueUsername()
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./files/users.txt", "r+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    read = getline(&line, &len, fp);
    printf("%s\n", line);
}

int createUser()
{
    // User input name, username, password
    char *name = (char *)malloc(sizeof(char) * 20),
         *username = (char *)malloc(sizeof(char) * 20),
         *password = (char *)malloc(sizeof(char) * 30);

    printf("Enter your name : ");
    gets(name);

    printf("Enter your username : ");
    gets(username);

    checkUniqueUsername(username);

    printf("Enter a password : ");
    gets(password);

    printf("Name : %s\nUsername : %s\nPassword : %s\n", name, username, password);

    // check username unique or not

    // if unique -> add new user to server li];
    // else  print user name is name is not unique return -1
}

int main()
{
    createUser();
}
