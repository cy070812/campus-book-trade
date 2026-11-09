#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDateEdit>
#include "book_core.h"
#include "common.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setCurrentUser(QString id, UserRole role);

private slots:
    void refreshBookTable();
    void refreshMyBooksTable();
    void refreshTradeRecordTable();
    void onPublishClicked();
    void onOffShelfClicked();
    void onBuyClicked();
    void onDeleteBookClicked();
    void onQueryBooks();
    void onResetQuery();
    void onSearchByNameForBuy();
    void onStatisticsClicked();
    void onTimeStatisticsClicked();
    void onExportClicked();
    void onBatchImport();
    void onBatchExport();
    void onClearDataClicked();
    void refreshUserTable();
    void onAddUser();
    void onDeleteUser();
    void onChangePassword();     // 新增
    void onToggleRole();         // 新增
    void onNavigationChanged(int index);

private:
    QString m_currentUserId;
    UserRole m_currentRole;

    QListWidget *navList;
    QStackedWidget *stackedWidget;
    QTableWidget *tableBooks;
    QTableWidget *myBooksTable;
    QTableWidget *tableTradeRecords;
    QTableWidget *tableUsers;

    QLineEdit *editBookName, *editIsbn, *editAuthor, *editPublisher, *editPrice, *editSellerName, *editPhone;
    QComboBox *comboCategory;

    QLineEdit *editBuyId, *editBuyByName;
    QPushButton *btnSearchBookByName;

    QComboBox *comboQueryType;
    QLineEdit *editQueryKeyword, *editPriceMin, *editPriceMax;
    QPushButton *btnQuery, *btnResetQuery;

    QDateEdit *dateStart, *dateEnd;
    QPushButton *btnTimeStats;
    QLabel *labelTimeSold, *labelTimeAmount, *labelCategory1, *labelCategory2, *labelCategory3, *labelCategory4;
    QLabel *labelTotal, *labelSold, *labelOffShelf, *labelTotalAmount;
    QPushButton *btnExport, *btnDeleteBook, *btnClearData;
    QPushButton *btnBatchImport, *btnBatchExport;

    QLineEdit *editNewUsername, *editNewPassword;
    QPushButton *btnAddUser, *btnDeleteUser;
    QPushButton *btnChangePwd, *btnToggleRole;   // 新增

    QLabel *labelUser;

    LinkList m_currentDisplayList;

    void initUI();
    void initTableHeader();
    void initMyBooksTableHeader();
    void initTradeRecordTableHeader();
    void initUserTableHeader();
    void applyPermissionControl();
    QString statusToString(BookStatus status);
    void displayBookList(LinkList list, QTableWidget *table, bool isMyBooksPage);
    void freeCurrentDisplayList();
};

#endif