#include "user_core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

UserList user_list = NULL;

void init_user_list() {
    user_list = (UserList)malloc(sizeof(UserNode));
    user_list->next = NULL;
}

void load_users_from_csv(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        generate_default_admin();
        return;
    }
    char line[256];
    fgets(line, sizeof(line), fp); // 跳过标题行
    while (fgets(line, sizeof(line), fp)) {
        User u;
        sscanf(line, "%[^,],%[^,],%d", u.username, u.password, &u.role);
        UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
        newNode->data = u;
        newNode->next = user_list->next;
        user_list->next = newNode;
    }
    fclose(fp);
}

void save_users_to_csv(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "username,password,role\n");
    UserNode* p = user_list->next;
    while (p) {
        fprintf(fp, "%s,%s,%d\n", p->data.username, p->data.password, p->data.role);
        p = p->next;
    }
    fclose(fp);
}

int add_user(const char* username, const char* password, int role) {
    if (find_user(username)) return -1;
    UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
    strcpy(newNode->data.username, username);
    strcpy(newNode->data.password, password);
    newNode->data.role = role;
    newNode->next = user_list->next;
    user_list->next = newNode;
    save_users_to_csv("users.csv");
    return 0;
}

int delete_user(const char* username) {
    UserNode* prev = user_list;
    while (prev->next) {
        if (strcmp(prev->next->data.username, username) == 0) {
            UserNode* toDel = prev->next;
            prev->next = toDel->next;
            free(toDel);
            save_users_to_csv("users.csv");
            return 0;
        }
        prev = prev->next;
    }
    return -1;
}

User* find_user(const char* username) {
    UserNode* p = user_list->next;
    while (p) {
        if (strcmp(p->data.username, username) == 0)
            return &p->data;
        p = p->next;
    }
    return NULL;
}

void free_user_list() {
    UserNode* p = user_list;
    while (p) {
        UserNode* q = p;
        p = p->next;
        free(q);
    }
    user_list = NULL;
}

void generate_default_admin() {
    add_user("admin", "admin123", 1);
    add_user("20240001", "123456", 0);
    add_user("20240002", "123456", 0);
}

// ========== 新增函数实现 ==========
int update_user_password(const char* username, const char* new_password) {
    UserNode* p = user_list->next;
    while (p) {
        if (strcmp(p->data.username, username) == 0) {
            strcpy(p->data.password, new_password);
            save_users_to_csv("users.csv");
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int update_user_role(const char* username, int new_role) {
    if (strcmp(username, "admin") == 0) {
        return -2;
    }
    UserNode* p = user_list->next;
    while (p) {
        if (strcmp(p->data.username, username) == 0) {
            p->data.role = new_role;
            save_users_to_csv("users.csv");
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int is_admin(const char* username) {
    return (strcmp(username, "admin") == 0);
}