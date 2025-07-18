#ifndef USER_H
#define USER_H

#include "string.h"

#define MAX_USERS 20
#define MAX_USERNAME 32
#define MAX_PASSWORD 32

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} User;

extern User users[MAX_USERS];
extern int user_count;
extern char logged_in_user[MAX_USERNAME];

void create_user(const char* username, const char* password);
bool check_password(const char* username, const char* password);
void init_user();
int user_exists(const char* username);

#endif
