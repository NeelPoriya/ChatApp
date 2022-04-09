#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/*
    Current features :
        1. Create & save a new User
        2. Get a user using username
        3. Print user details using username
*/

enum Parameters
{
    Name = 0,
    Username = 1,
    Password = 2,
    Age = 3,
    LastLogin = 4,
    Status = 5
};

struct User
{
    char *name;
    char *username;
    char *password;
    char *date_of_birth;
    char *last_login;
    char *status;
    int age;
};

int checkUniqueUsername(char *username)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./files/users.txt", "r+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // read every line of users.txt file
    while ((read = getline(&line, &len, fp)) != -1)
    {
        // if we find a new line (meaning that file is over) we break
        if (!strcmp(line, "\n"))
            break;

        // extracting username from each line
        char *f_username = strtok(line, ";");
        for (int i = 0; i < Username; ++i)
            f_username = strtok(NULL, ";");

        // check if username is same or not
        if (!strcmp(username, f_username))
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

char *convertToString(struct User *user)
{
    char *result = malloc(sizeof(char) * 200);
    memset(result, '\0', sizeof(result));
    char age[4];

    sprintf(age, "%d", user->age);

    strcat(result, user->name);
    strcat(result, ";");
    strcat(result, user->username);
    strcat(result, ";");
    strcat(result, user->password);
    strcat(result, ";");
    strcat(result, user->date_of_birth);
    strcat(result, ";");
    strcat(result, user->last_login);
    strcat(result, ";");
    strcat(result, age);
    strcat(result, ";");
    strcat(result, user->status);
    strcat(result, "\n");

    free(user->name);
    free(user->username);
    free(user->password);
    free(user->last_login);
    free(user->date_of_birth);
    free(user->status);
    free(user);

    return result;
}

void saveUser(char *user_string)
{
    FILE *fp = fopen("./files/users.txt", "a");
    fputs(user_string, fp);
    fclose(fp);
}

void debug(struct User *user)
{
    if (user == NULL)
    {
        printf("\t\t\tNo User found\n");
        return;
    }
    printf("\tName\t\t:\t%s\n\tUsername\t:\t%s\n\tPassword\t:\t%s\n\tDate of Birth\t:\t%s\n\tLast-login\t:\t%s\n\tStatus\t\t:\t%s\n\tAge\t\t:\t%d\n", user->name, user->username, user->password, user->date_of_birth, user->last_login, user->status, user->age);
}

struct User *convertToStruct(char *line)
{
    struct User *user = (struct User *)malloc(sizeof(struct User));
    user->name = (char *)malloc(sizeof(char) * 20),
    user->username = (char *)malloc(sizeof(char) * 20),
    user->password = (char *)malloc(sizeof(char) * 30),
    user->date_of_birth = (char *)malloc(sizeof(char) * 10),
    user->status = (char *)malloc(sizeof(char) * 50),
    user->last_login = (char *)malloc(sizeof(char) * 10);

    line = strtok(line, ";");
    strcpy(user->name, line);

    line = strtok(NULL, ";");
    strcpy(user->username, line);

    line = strtok(NULL, ";");
    strcpy(user->password, line);

    line = strtok(NULL, ";");
    strcpy(user->date_of_birth, line);

    line = strtok(NULL, ";");
    strcpy(user->last_login, line);

    line = strtok(NULL, ";");
    user->age = atoi(line);

    line = strtok(NULL, ";");
    strcpy(user->status, line);
    // change last character from new line to null.
    user->status[strlen(user->status) - 1] = '\0';

    return user;
}

struct User *getUser(char *username)
{
    printf("User name : %s\n", username);
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp = fopen("./files/users.txt", "r+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        // if we find a new line (meaning that file is over) we break
        if (!strcmp(line, "\n"))
            break;
        char user_str[200];
        strcpy(user_str, line);

        // extracting username from each line
        char *f_username = strtok(line, ";");
        for (int i = 0; i < Username; ++i)
            f_username = strtok(NULL, ";");

        // check if username is same or not
        if (!strcmp(username, f_username))
        {
            return convertToStruct(user_str);
            fclose(fp);
        }
    }
    return NULL;
}

int checkUser(char *username, char *password)
{
    struct User *user = getUser(username);
    if (user == NULL)
    {
        return 0;
    }
    if (strcmp(password, user->password))
    {
        return 0;
    }
    return 1;
}

struct User *createUser()
{
    // Ask for user parameters
    struct User *newUser = (struct User *)malloc(sizeof(struct User));
    newUser->name = (char *)malloc(sizeof(char) * 20),
    newUser->username = (char *)malloc(sizeof(char) * 20),
    newUser->password = (char *)malloc(sizeof(char) * 30),
    newUser->date_of_birth = (char *)malloc(sizeof(char) * 10),
    newUser->status = (char *)malloc(sizeof(char) * 50),
    newUser->last_login = (char *)malloc(sizeof(char) * 10);

    printf("Enter your name : ");
    scanf("%[^\n]s", newUser->name), getchar();

    printf("Enter your username : ");
    scanf("%[^\n]s", newUser->username), getchar();

    // check username unique or not
    if (checkUniqueUsername(newUser->username))
    {
        printf("\nGiven username is already taken.\nPlease try again\n\n");
        return NULL;
    }

    printf("Enter password : ");
    scanf("%[^\n]s", newUser->password), getchar();

    printf("Enter date of birth (DD-MM-YYYY) : ");
    scanf("%[^\n]s", newUser->date_of_birth), getchar();

    // check if date is valid.
    // checkDateOfBirth(newUser->date_of_birth)

    printf("Enter your status : ");
    scanf("%[^\n]s", newUser->status), getchar();

    strcpy(newUser->last_login, "online");

    // calculate age.
    // newUser->age = calculateAge(newUser->date_of_birth);
    newUser->age = 10;

    return newUser;
}

void printUser(struct User *user)
{
    printf("/------------------------------------------------------------/\n");
    printf("\t\t\tUSER PROFILE\n");
    printf("/------------------------------------------------------------/\n");

    debug(user);
}

char *getUserProfileString(struct User *user)
{
    char *result = (char *)malloc((sizeof(char)) * 1000);
    if (user == NULL)
    {
        strcpy(result, "\t\t\tNo User found\n");
        return result;
    }
    sprintf(result, "\tName\t\t:\t%s\n\tUsername\t:\t%s\n\tPassword\t:\t%s\n\tDate of Birth\t:\t%s\n\tLast-login\t:\t%s\n\tStatus\t\t:\t%s\n\tAge\t\t:\t%d\n", user->name, user->username, user->password, user->date_of_birth, user->last_login, user->status, user->age);
    return result;
}

// int main()
// {
//     // createUser();
//     struct User *usr = getUser("remeshbhoi");
//     printUser(usr);
// }
