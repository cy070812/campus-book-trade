#ifndef BOOK_CORE_H
#define BOOK_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    STATUS_ON_SALE = 0,
    STATUS_SOLD = 1,
    STATUS_OFF_SHELF = 2
} BookStatus;

typedef struct Book {
    int book_id;
    char isbn[20];
    char book_name[100];
    char author[50];
    char publisher[50];
    float price;
    char seller_id[20];
    char seller_name[20];
    char seller_phone[20];
    BookStatus status;
    char publish_time[20];
    char buyer_id[20];
    char trade_time[20];
    int category;
} Book;

typedef struct Node {
    Book data;
    struct Node* next;
} Node, *LinkList;

#ifdef __cplusplus
extern "C" {
#endif

extern LinkList book_list;
extern int next_book_id;

void init_list();
void load_from_csv(const char* filename);
void save_to_csv(const char* filename);
int add_book(Book book);
int sell_book(int book_id, const char* buyer_id);
int off_shelf_book(int book_id, const char* seller_id);
int delete_book(int book_id);
Node* query_book_by_id(int book_id);
Node* get_all_trade_records();
Node* query_books_by_condition(int type, const char* keyword, float min_price, float max_price);
void get_statistics(int* total, int* sold, int* off_shelf, float* total_amount);
void get_statistics_by_time(const char* start_time, const char* end_time,
                            int* sold_count, float* total_amount,
                            int category_sales[5]);
void get_current_time(char* time_str);
void free_list();
void free_result_list(LinkList list);

#ifdef __cplusplus
}
#endif

#endif