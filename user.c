#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

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

int age(char date[])
{
    time_t s, val = 1;
    struct tm *current_time;

    s = time(NULL);

    current_time = localtime(&s);

    char year[4];

    for (int i = 6; i < 10; i++)
    {
        year[i - 6] = date[i];
    }

    int b_year = atoi(year);

    int curr_year = (current_time->tm_year + 1900);

    return curr_year - b_year;
}

int isleap(int y)
{
    if ((y % 4 == 0) && (y % 100 != 0) && (y % 400 == 0))
        return 1;
    else
        return 0;
}

int datevalid(char *date)
{
    time_t s, val = 1;
    struct tm *current_time;

    s = time(NULL);

    current_time = localtime(&s);

    int y = 0, m = 0, d = 0, min_yr = 1800, max_yr = 9999;

    int current_month = current_time->tm_mon+1;
    int current_date = current_time->tm_mday; 
    int current_year = current_time->tm_year+1900;

    

    if(strlen(date) != 10)
    {
        printf("\nPlease enter date in DD-MM-YYYY form \n");
        return 0;
    }

    for (int i = 0; i < 10; i++)
    {
        if (i == 0 || i == 1)
        {
            d = d * 10 + date[i] - '0';
        }

        if (i == 3 || i == 4)
        {
            m = m * 10 + date[i] - '0';
        }

        if (i >= 6 && i <= 9)
        {
            y = y * 10 + date[i] - '0';
        }
    }

    if((y>current_year) || (y>=current_year && m>current_month) || (y>=current_year && m>=current_month && d>current_date))
    {
        printf("Please enter date in DD-MM-YYYY form \n");
        return 0;
    }

    if (y < min_yr || y > max_yr)
        return 0;
    if (m < 1 || m > 12)
        return 0;
    if (d < 1 || d > 31)
        return 0;

    if (m == 2)
    {
        if (isleap(y))
        {
            if (d <= 29)
                return 1;
            else
                return 0;
        }
    }

    if (m == 4 || m == 6 || m == 9 || m == 11)
        if (d <= 30)
            return 1;
        else
            return 0;
    return 1;
}

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
    printf("\tName\t\t:\t%s\n\tUsername\t:\t%s\n\tPassword\t:\t%s\n\tDate of Birth\t:\t%s\n\tLast-login\t:\t%s\t\nStatus\t\t:\t%s\n\tAge\t\t:\t%d\n", user->name, user->username, user->password, user->date_of_birth, user->last_login, user->status, user->age);
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

void copyFile(char *source, char *destination)
{
    FILE *fptr1, *fptr2;
    char filename[100], c;

    // Open one file for reading
    fptr1 = fopen(source, "r");
    if (fptr1 == NULL)
    {
        printf("Cannot open file %s \n", filename);
        exit(0);
    }

    // Open another file for writing
    fptr2 = fopen(destination, "w");
    if (fptr2 == NULL)
    {
        printf("Cannot open file %s \n", filename);
        exit(0);
    }

    // Read contents from file
    c = fgetc(fptr1);
    while (c != EOF)
    {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }
    fclose(fptr1);
    fclose(fptr2);
}

char *getSession(char *username)
{
    FILE *fp = fopen("./files/session.txt", "r");
    char *usrname, *t;
    char buf[1024];

    while (fgets(buf, sizeof(buf), fp))
    {
        usrname = strtok(buf, ";");
        t = strtok(NULL, ";");

        if (strcmp(usrname, username) == 0)
        {
            return t;
        }
    }
    printf("User not found!\n");
    return NULL;
}

// update 0 --> online
// update 1 --> offile (last login date)
void updateSession(char *username, int update)
{
    copyFile("./files/session.txt", "./files/temp.txt");
    FILE *fp = fopen("./files/temp.txt", "r");
    FILE *fp2 = fopen("./files/session.txt", "w");
    char buf[1024];
    char addThis[50];
    while (fgets(buf, sizeof(buf), fp))
    {
        char *usr, *t;
        usr = strtok(buf, ";");
        t = strtok(NULL, ";");

        if (strcmp(usr, username) == 0)
        {
            if (update == 1)
            {
                char *current_time = malloc(sizeof(char) * 20);
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(current_time, "%02d-%02d-%d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
                sprintf(addThis, "%s;%s\n", username, current_time);
                free(current_time);
            }
            else
            {
                sprintf(addThis, "%s;%s\n", username, "online");
            }
            fputs(addThis, fp2);
        }
        else
        {
            fputs(usr, fp2);
            fputs(";", fp2);
            fputs(t, fp2);
        }
    }
    remove("./files/temp.txt");
    fclose(fp);
    fclose(fp2);
}

void addSession(char *username)
{
    FILE *fp = fopen("./files/session.txt", "a");
    char addThis[50];
    sprintf(addThis, "%s;%s\n", username, "online");
    fputs(addThis, fp);
    fclose(fp);
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

    do{
        printf("Enter your name : ");
        scanf("%[^\n]s", newUser->name), getchar();
    }while(newUser->name == NULL || strlen(newUser->name) == 0 || strlen(newUser->name) > 20);

    while(1){
        int leave = 0;
        printf("Enter your username : ");
        scanf("%[^\n]s", newUser->username), getchar();
        if(newUser->username == NULL || strlen(newUser->username) == 0 || strlen(newUser->username) > 30)
            continue;
        for(int i = 0; i < strlen(newUser->username); ++i){
            if(newUser->username[i] == ' '){
                printf("Please provide username with no spaces.\n");
                leave = 1;
                break;
            }
        }

        if(!leave) break;
    }

    // check username unique or not
    if (checkUniqueUsername(newUser->username))
    {
        printf("\nGiven username is already taken.\nPlease try again\n\n");
        return NULL;
    }

    addSession(newUser->username);

    do{
        printf("Enter password : ");
        scanf("%[^\n]s", newUser->password), getchar();
    }while(newUser->password == NULL || strlen(newUser->password) == 0 || strlen(newUser->password) > 30);

    do
    {
        printf("Enter date of birth (DD-MM-YYYY) : ");
        scanf("%[^\n]s", newUser->date_of_birth), getchar();
    } while (!datevalid(newUser->date_of_birth));

    // check if date is valid.

    do{
        printf("Enter your status : ");
        scanf("%[^\n]s", newUser->status), getchar();
    }while(newUser->status == NULL || strlen(newUser->status) == 0);

    strcpy(newUser->last_login, "online");

    // calculate age.
    newUser->age = age(newUser->date_of_birth);

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
    char *final_result = (char*) malloc((sizeof(char)) * 2000);
    char *result = (char *)malloc((sizeof(char)) * 1000);
    if (user == NULL)
    {
        strcpy(result, "\t\t\tNo User found\n");
        return result;
    }
    FILE* fptr = fopen("./profile.txt", "r");
    
    char profile_text[1000];
    while(fgets(profile_text, sizeof(profile_text), fptr)){
    	strcat(final_result, profile_text);
    }
    printf("Get User : %s", getSession(user->username));
    sprintf(result, "\n\tName\t\t:\t%s\n\tUsername\t:\t%s\n\tPassword\t:\t******\n\tDate of Birth\t:\t%s\n\tLast-login\t:\t%s\tStatus\t\t:\t%s\n\tAge\t\t:\t%d\n", user->name, user->username, user->date_of_birth, getSession(user->username), user->status, user->age);
    
    strcat(final_result, result);
    return final_result;
}
