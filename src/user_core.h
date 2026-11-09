#ifndef USER_CORE_H
#define USER_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct User {
    char username[50];
    char password[50];
    int role;
} User;

typedef struct UserNode {
    User data;
    struct UserNode* next;
} UserNode, *UserList;

#ifdef __cplusplus
extern "C" {
#endif

extern UserList user_list;

void init_user_list();
void load_users_from_csv(const char* filename);
void save_users_to_csv(const char* filename);
int add_user(const char* username, const char* password, int role);
int delete_user(const char* username);
User* find_user(const char* username);
void free_user_list();
void generate_default_admin();

int update_user_password(const char* username, const char* new_password);
int update_user_role(const char* username, int new_role);
int is_admin(const char* username);

#ifdef __cplusplus
}
#endif

#endif