#include "book_core.h"

LinkList book_list = NULL;
int next_book_id = 10001;

void init_list() {
    book_list = (LinkList)malloc(sizeof(Node));
    book_list->next = NULL;
}

void get_current_time(char* time_str) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(time_str, "%04d-%02d-%02d %02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);
}

int add_book(Book book) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return -1;
    book.book_id = next_book_id++;
    book.status = STATUS_ON_SALE;
    get_current_time(book.publish_time);
    strcpy(book.buyer_id, "");
    strcpy(book.trade_time, "");
    new_node->data = book;
    new_node->next = book_list->next;
    book_list->next = new_node;
    save_to_csv("books.csv");
    return 0;
}

int sell_book(int book_id, const char* buyer_id) {
    Node* p = book_list->next;
    while (p) {
        if (p->data.book_id == book_id) {
            if (p->data.status != STATUS_ON_SALE) return -2;
            p->data.status = STATUS_SOLD;
            strcpy(p->data.buyer_id, buyer_id);
            get_current_time(p->data.trade_time);
            save_to_csv("books.csv");
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int off_shelf_book(int book_id, const char* seller_id) {
    Node* p = book_list->next;
    while (p) {
        if (p->data.book_id == book_id) {
            if (strcmp(p->data.seller_id, seller_id) != 0) return -2;
            if (p->data.status != STATUS_ON_SALE) return -3;
            p->data.status = STATUS_OFF_SHELF;
            save_to_csv("books.csv");
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int delete_book(int book_id) {
    Node* p = book_list;
    while (p->next) {
        if (p->next->data.book_id == book_id) {
            Node* to_delete = p->next;
            p->next = to_delete->next;
            free(to_delete);
            save_to_csv("books.csv");
            return 0;
        }
        p = p->next;
    }
    return -1;
}

Node* get_all_trade_records() {
    LinkList result = (LinkList)malloc(sizeof(Node));
    result->next = NULL;
    Node* tail = result;
    Node* p = book_list->next;
    while (p) {
        if (p->data.status == STATUS_SOLD) {
            Node* new_node = (Node*)malloc(sizeof(Node));
            new_node->data = p->data;
            new_node->next = NULL;
            tail->next = new_node;
            tail = new_node;
        }
        p = p->next;
    }
    return result;
}

Node* query_book_by_id(int book_id) {
    Node* p = book_list->next;
    while (p) {
        if (p->data.book_id == book_id) return p;
        p = p->next;
    }
    return NULL;
}

Node* query_books_by_condition(int type, const char* keyword, float min_price, float max_price) {
    LinkList result = (LinkList)malloc(sizeof(Node));
    result->next = NULL;
    Node* tail = result;
    Node* p = book_list->next;
    while (p) {
        int match = 0;
        switch (type) {
            case 1: if (strstr(p->data.book_name, keyword)) match = 1; break;
            case 2: if (strcmp(p->data.author, keyword) == 0) match = 1; break;
            case 3: if (strcmp(p->data.isbn, keyword) == 0) match = 1; break;
            case 4: if (strcmp(p->data.seller_id, keyword) == 0) match = 1; break;
            case 5: if (p->data.status == atoi(keyword)) match = 1; break;
            case 6: if (p->data.price >= min_price && p->data.price <= max_price) match = 1;
    break;
        }
        if (match) {
            Node* new_node = (Node*)malloc(sizeof(Node));
            new_node->data = p->data;
            new_node->next = NULL;
            tail->next = new_node;
            tail = new_node;
        }
        p = p->next;
    }
    return result;
}

void get_statistics(int* total, int* sold, int* off_shelf, float* total_amount) {
    *total = *sold = *off_shelf = 0;
    *total_amount = 0.0f;
    Node* p = book_list->next;
    while (p) {
        (*total)++;
        if (p->data.status == STATUS_SOLD) {
            (*sold)++;
            *total_amount += p->data.price;
        } else if (p->data.status == STATUS_OFF_SHELF) {
            (*off_shelf)++;
        }
        p = p->next;
    }
}

void get_statistics_by_time(const char* start_time, const char* end_time,
                            int* sold_count, float* total_amount,
                            int category_sales[5]) {
    *sold_count = 0;
    *total_amount = 0.0f;
    for (int i = 1; i <= 4; i++) category_sales[i] = 0;
    Node* p = book_list->next;
    while (p) {
        if (p->data.status == STATUS_SOLD && strlen(p->data.trade_time) > 0) {
            if (strcmp(p->data.trade_time, start_time) >= 0 && strcmp(p->data.trade_time, end_time) <= 0) {
                (*sold_count)++;
                *total_amount += p->data.price;
                int cat = p->data.category;
                if (cat >= 1 && cat <= 4) category_sales[cat]++;
            }
        }
        p = p->next;
    }
}

void save_to_csv(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    fprintf(fp, "book_id,isbn,book_name,author,publisher,price,seller_id,seller_name,seller_phone,status,publish_time,buyer_id,trade_time,category\n");
    Node* p = book_list->next;
    while (p) {
        Book* b = &p->data;
        fprintf(fp, "%d,%s,%s,%s,%s,%.2f,%s,%s,%s,%d,%s,%s,%s,%d\n",
                b->book_id, b->isbn, b->book_name, b->author, b->publisher,
                b->price, b->seller_id, b->seller_name, b->seller_phone,
                b->status, b->publish_time, b->buyer_id, b->trade_time, b->category);
        p = p->next;
    }
    fclose(fp);
}

void load_from_csv(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        // 文件不存在，不生成样例数据
        return;
    }
    char line[1024];
    fgets(line, sizeof(line), fp);
    int max_id = 10000;
    while (fgets(line, sizeof(line), fp)) {
        Book book;
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%f,%[^,],%[^,],%[^,],%d,%[^,],%[^,],%[^,],%d",
               &book.book_id, book.isbn, book.book_name, book.author, book.publisher,
               &book.price, book.seller_id, book.seller_name, book.seller_phone,
               (int*)&book.status, book.publish_time, book.buyer_id, book.trade_time, &book.category);
        if (book.book_id > max_id) max_id = book.book_id;
        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->data = book;
        new_node->next = book_list->next;
        book_list->next = new_node;
    }
    fclose(fp);
    if (max_id >= 10001) next_book_id = max_id + 1;
    else next_book_id = 10001;
}

void free_list() {
    Node* p = book_list;
    while (p) {
        Node* q = p;
        p = p->next;
        free(q);
    }
    book_list = NULL;
}

void free_result_list(LinkList list) {
    if (!list) return;
    Node* p = list;
    while (p) {
        Node* q = p;
        p = p->next;
        free(q);
    }
}