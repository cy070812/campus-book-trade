#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include "user_core.h"

// ========== 样式宏定义（黑白圆角风格） ==========
// 主要按钮样式：黑底白字，圆角12px
#define PRIMARY_BUTTON_STYLE \
"QPushButton {" \
    "   background-color: #1a1a1a;" \
    "   color: #ffffff;" \
    "   border: none;" \
    "   border-radius: 12px;" \
    "   padding: 8px 20px;" \
    "   font-size: 14px;" \
    "   font-weight: 500;" \
    "}" \
    "QPushButton:hover { background-color: #333333; }"

// 次要按钮样式：白底灰边，黑字
#define SECONDARY_BUTTON_STYLE \
    "QPushButton {" \
    "   background-color: #f0f0f0;" \
    "   color: #1a1a1a;" \
    "   border: 1px solid #cccccc;" \
    "   border-radius: 10px;" \
    "   padding: 6px 16px;" \
    "   font-size: 13px;" \
    "}" \
    "QPushButton:hover { background-color: #e0e0e0; }"

// 危险按钮样式：深灰底白字
#define DANGER_BUTTON_STYLE \
    "QPushButton {" \
    "   background-color: #2c2c2c;" \
    "   color: #ffffff;" \
    "   border: none;" \
    "   border-radius: 10px;" \
    "   padding: 6px 16px;" \
    "}" \
    "QPushButton:hover { background-color: #444444; }"

// 大号主要按钮样式（用于发布、购买等）
#define PRIMARY_BUTTON_STYLE_LARGE \
    "QPushButton {" \
    "   background-color: #1a1a1a;" \
    "   color: #ffffff;" \
    "   border: none;" \
    "   border-radius: 14px;" \
    "   padding: 10px 24px;" \
    "   font-size: 16px;" \
    "   font-weight: 500;" \
    "}" \
    "QPushButton:hover { background-color: #333333; }"

    // 构造函数：初始化界面、数据链表、表格等
    MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentRole(ROLE_USER), m_currentDisplayList(nullptr) {
    initUI();                           // 创建用户界面
    init_list();                        // 初始化图书链表
    load_from_csv("books.csv");         // 从CSV加载图书数据
    initTableHeader();                  // 设置图书浏览表格表头
    initMyBooksTableHeader();           // 设置“我的图书”表格表头
    initTradeRecordTableHeader();       // 设置交易记录表格表头
    refreshBookTable();                 // 刷新图书浏览表格
    refreshMyBooksTable();              // 刷新“我的图书”表格
    refreshUserTable();                 // 刷新人员管理表格
}

// 析构函数：释放动态分配的内存
MainWindow::~MainWindow() {
    freeCurrentDisplayList();           // 释放查询结果链表
    free_list();                        // 释放图书链表
    free_user_list();                   // 释放用户链表
}

// 设置当前登录用户
void MainWindow::setCurrentUser(QString id, UserRole role) {
    m_currentUserId = id;
    m_currentRole = role;
    QString roleText = (role == ROLE_ADMIN) ? "管理员" : "普通用户";
    labelUser->setText(QString("当前用户：%1 (%2)").arg(id).arg(roleText));
    applyPermissionControl();           // 根据角色控制界面元素可见性
    refreshMyBooksTable();              // 刷新“我的图书”（普通用户只显示自己的）
    refreshBookTable();                 // 刷新图书浏览（显示所有图书）
}

// 根据角色权限控制菜单项和按钮的显示/隐藏
void MainWindow::applyPermissionControl() {
    bool isAdmin = (m_currentRole == ROLE_ADMIN);
    // 控制左侧导航栏中的“交易记录”和“人员管理”项
    if (navList->count() >= 7) {
        navList->item(5)->setHidden(!isAdmin);
        navList->item(6)->setHidden(!isAdmin);
    }
    // 如果当前选中的是管理员专属页面且不是管理员，则跳转到图书浏览页
    if (!isAdmin && (navList->currentRow() == 5 || navList->currentRow() == 6)) {
        navList->setCurrentRow(0);
        onNavigationChanged(0);
    }
    // 控制管理员专属按钮的可见性
    if (btnBatchImport) btnBatchImport->setVisible(isAdmin);
    if (btnBatchExport) btnBatchExport->setVisible(isAdmin);
    if (btnDeleteBook) btnDeleteBook->setVisible(isAdmin);
    if (btnClearData) btnClearData->setVisible(isAdmin);
    if (btnChangePwd) btnChangePwd->setVisible(isAdmin);
    if (btnToggleRole) btnToggleRole->setVisible(isAdmin);
    if (btnDeleteUser) btnDeleteUser->setVisible(isAdmin);
}

// 初始化整个用户界面
void MainWindow::initUI() {
    // 设置主窗口标题、固定大小、全局样式
    setWindowTitle("校园二手书交易管理系统");
    setFixedSize(1550, 950);
    setStyleSheet(
        "QMainWindow { background-color: #fafafa; }"
        "QWidget { background-color: #fafafa; }"
        "QLabel { color: #1a1a1a; }"
        "QLineEdit, QComboBox, QDateEdit {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 10px;"
        "   padding: 8px 12px;"
        "   background-color: #ffffff;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: #1a1a1a; }"
        );

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 顶部用户信息栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    labelUser = new QLabel("当前用户：未登录", this);
    labelUser->setStyleSheet(
        "background-color: #ffffff;"
        "border: 1px solid #e0e0e0;"
        "border-radius: 20px;"
        "padding: 8px 20px;"
        "font-size: 14px;"
        "font-weight: 500;"
        );
    topLayout->addWidget(labelUser);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // 主体：左侧导航栏 + 右侧内容区
    QHBoxLayout *horLayout = new QHBoxLayout();
    horLayout->setSpacing(15);

    // ========== 左侧导航栏 ==========
    navList = new QListWidget(this);
    navList->setFixedWidth(180);
    navList->setStyleSheet(
        "QListWidget { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 16px; outline: 0; }"
        "QListWidget::item { padding: 12px 20px; color: #333333; font-size: 14px; border-radius: 10px; margin: 2px 8px; }"
        "QListWidget::item:selected { background-color: #1a1a1a; color: #ffffff; }"
        "QListWidget::item:hover { background-color: #f0f0f0; }"
        );
    navList->addItem("📖 图书浏览");
    navList->addItem("📝 发布图书");
    navList->addItem("💸 购买图书");
    navList->addItem("📚 我的图书");
    navList->addItem("📈 统计报表");
    navList->addItem("📜 交易记录");
    navList->addItem("👥 人员管理");
    connect(navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);

    // 右侧堆叠窗口（每个标签页对应一个页面）
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setStyleSheet("QStackedWidget { background-color: #ffffff; border-radius: 16px; }");

    // -------------------- 页面0：图书浏览 --------------------
    QWidget *pageBrowse = new QWidget();
    QVBoxLayout *browseLayout = new QVBoxLayout(pageBrowse);
    browseLayout->setContentsMargins(25,25,25,25);
    browseLayout->setSpacing(20);

    // 多条件查询分组框
    QGroupBox *queryGroup = new QGroupBox("🔍 多条件查询");
    queryGroup->setStyleSheet("QGroupBox { border: 1px solid #e0e0e0; border-radius: 14px; margin-top: 14px; }"
                              "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 8px; }");
    QGridLayout *queryLayout = new QGridLayout(queryGroup);
    comboQueryType = new QComboBox();
    comboQueryType->addItems({"按书名关键字","按作者","按ISBN","按价格区间","按卖家学号","按交易状态"});
    editQueryKeyword = new QLineEdit(); editQueryKeyword->setPlaceholderText("输入关键字");
    editPriceMin = new QLineEdit(); editPriceMin->setPlaceholderText("最低价");
    editPriceMax = new QLineEdit(); editPriceMax->setPlaceholderText("最高价");
    editPriceMin->setVisible(false); editPriceMax->setVisible(false);
    btnQuery = new QPushButton("查询"); btnResetQuery = new QPushButton("显示全部");
    btnQuery->setStyleSheet(PRIMARY_BUTTON_STYLE);
    btnResetQuery->setStyleSheet(SECONDARY_BUTTON_STYLE);
    connect(btnQuery, &QPushButton::clicked, this, &MainWindow::onQueryBooks);
    connect(btnResetQuery, &QPushButton::clicked, this, &MainWindow::onResetQuery);
    connect(comboQueryType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int idx){
        bool isPrice = (idx == 3);
        editQueryKeyword->setVisible(!isPrice);
        editPriceMin->setVisible(isPrice);
        editPriceMax->setVisible(isPrice);
    });
    queryLayout->addWidget(comboQueryType,0,0);
    queryLayout->addWidget(editQueryKeyword,0,1);
    queryLayout->addWidget(editPriceMin,0,1);
    queryLayout->addWidget(editPriceMax,0,2);
    queryLayout->addWidget(btnQuery,0,3);
    queryLayout->addWidget(btnResetQuery,0,4);
    browseLayout->addWidget(queryGroup);

    // 管理员删除图书按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnDeleteBook = new QPushButton("🗑️ 删除选中图书（管理员）");
    btnDeleteBook->setStyleSheet(DANGER_BUTTON_STYLE);
    connect(btnDeleteBook, &QPushButton::clicked, this, &MainWindow::onDeleteBookClicked);
    btnLayout->addStretch();
    btnLayout->addWidget(btnDeleteBook);
    browseLayout->addLayout(btnLayout);

    // 图书表格（包含ISBN列）
    tableBooks = new QTableWidget(this);
    tableBooks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableBooks->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableBooks->setAlternatingRowColors(true);
    tableBooks->setStyleSheet(
        "QTableWidget { gridline-color: #e0e0e0; selection-background-color: #e8e8e8; border-radius: 12px; }"
        "QTableWidget::item:alternate { background-color: #f8f8f8; }"
        "QHeaderView::section { background-color: #f0f0f0; padding: 8px; border: none; font-weight: 600; }"
        );
    browseLayout->addWidget(tableBooks);
    stackedWidget->addWidget(pageBrowse);

    // -------------------- 页面1：发布图书 --------------------
    QWidget *pagePublish = new QWidget();
    QFormLayout *publishLayout = new QFormLayout(pagePublish);
    publishLayout->setSpacing(20);
    publishLayout->setContentsMargins(60,50,60,50);
    editBookName = new QLineEdit(); editIsbn = new QLineEdit(); editAuthor = new QLineEdit();
    editPublisher = new QLineEdit(); editPrice = new QLineEdit(); editSellerName = new QLineEdit();
    editPhone = new QLineEdit(); comboCategory = new QComboBox();
    comboCategory->addItems({"教材","教辅","课外读物","其他"});
    QString inputStyle = "QLineEdit, QComboBox { border: 1px solid #cccccc; border-radius: 10px; padding: 8px 12px; }";
    editBookName->setStyleSheet(inputStyle);
    editIsbn->setStyleSheet(inputStyle);
    editAuthor->setStyleSheet(inputStyle);
    editPublisher->setStyleSheet(inputStyle);
    editPrice->setStyleSheet(inputStyle);
    editSellerName->setStyleSheet(inputStyle);
    editPhone->setStyleSheet(inputStyle);
    comboCategory->setStyleSheet(inputStyle);
    publishLayout->addRow("📕 书名：", editBookName);
    publishLayout->addRow("🔢 ISBN：", editIsbn);
    publishLayout->addRow("✍️ 作者：", editAuthor);
    publishLayout->addRow("🏢 出版社：", editPublisher);
    publishLayout->addRow("💰 售价（元）：", editPrice);
    publishLayout->addRow("👤 卖家姓名：", editSellerName);
    publishLayout->addRow("📞 联系电话：", editPhone);
    publishLayout->addRow("📚 图书分类：", comboCategory);
    QPushButton *btnPublish = new QPushButton("✨ 发布图书");
    btnPublish->setFixedHeight(48);
    btnPublish->setStyleSheet(PRIMARY_BUTTON_STYLE_LARGE);
    connect(btnPublish, &QPushButton::clicked, this, &MainWindow::onPublishClicked);
    publishLayout->addRow(btnPublish);
    stackedWidget->addWidget(pagePublish);

    // -------------------- 页面2：购买图书 --------------------
    QWidget *pageBuy = new QWidget();
    QVBoxLayout *buyLayout = new QVBoxLayout(pageBuy);
    buyLayout->setContentsMargins(60,60,60,60);
    buyLayout->setSpacing(25);
    QGroupBox *buyGroup = new QGroupBox("🛒 购买方式");
    buyGroup->setStyleSheet(queryGroup->styleSheet());
    QFormLayout *buyForm = new QFormLayout(buyGroup);
    editBuyId = new QLineEdit(); editBuyId->setPlaceholderText("输入图书编号直接购买");
    editBuyId->setStyleSheet(inputStyle);
    buyForm->addRow("🔖 图书编号：", editBuyId);
    QHBoxLayout *nameSearchLayout = new QHBoxLayout();
    editBuyByName = new QLineEdit(); editBuyByName->setPlaceholderText("输入书名关键字查找");
    editBuyByName->setStyleSheet(inputStyle);
    btnSearchBookByName = new QPushButton("🔎 查找图书");
    btnSearchBookByName->setStyleSheet(SECONDARY_BUTTON_STYLE);
    connect(btnSearchBookByName, &QPushButton::clicked, this, &MainWindow::onSearchByNameForBuy);
    nameSearchLayout->addWidget(editBuyByName);
    nameSearchLayout->addWidget(btnSearchBookByName);
    buyForm->addRow("📖 按书名查找：", nameSearchLayout);
    QPushButton *btnBuy = new QPushButton("✅ 确认购买");
    btnBuy->setFixedHeight(48);
    btnBuy->setStyleSheet(PRIMARY_BUTTON_STYLE_LARGE);
    connect(btnBuy, &QPushButton::clicked, this, &MainWindow::onBuyClicked);
    buyLayout->addWidget(buyGroup);
    buyLayout->addWidget(btnBuy);
    buyLayout->addStretch();
    stackedWidget->addWidget(pageBuy);

    // -------------------- 页面3：我的图书 --------------------
    QWidget *pageMy = new QWidget();
    QVBoxLayout *myLayout = new QVBoxLayout(pageMy);
    myLayout->setContentsMargins(25,25,25,25);
    myLayout->setSpacing(15);
    QPushButton *btnOffShelfMy = new QPushButton("⬇️ 下架选中的图书（从下方表格选择）");
    btnOffShelfMy->setStyleSheet(DANGER_BUTTON_STYLE);
    btnOffShelfMy->setFixedHeight(45);
    connect(btnOffShelfMy, &QPushButton::clicked, this, &MainWindow::onOffShelfClicked);
    myLayout->addWidget(btnOffShelfMy);
    myBooksTable = new QTableWidget(this);
    myBooksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myBooksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myBooksTable->setAlternatingRowColors(true);
    myBooksTable->setStyleSheet(tableBooks->styleSheet());
    myLayout->addWidget(myBooksTable);
    stackedWidget->addWidget(pageMy);

    // -------------------- 页面4：统计报表 --------------------
    QWidget *pageStats = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(pageStats);
    statsLayout->setContentsMargins(40,40,40,40);
    statsLayout->setSpacing(25);

    // 全局统计
    QGroupBox *globalGroup = new QGroupBox("📊 全局统计");
    globalGroup->setStyleSheet(queryGroup->styleSheet());
    QFormLayout *globalForm = new QFormLayout(globalGroup);
    labelTotal = new QLabel("0"); labelSold = new QLabel("0"); labelOffShelf = new QLabel("0"); labelTotalAmount = new QLabel("0.00");
    labelTotal->setStyleSheet("font-size:16px; font-weight:bold;");
    labelSold->setStyleSheet(labelTotal->styleSheet());
    labelOffShelf->setStyleSheet(labelTotal->styleSheet());
    labelTotalAmount->setStyleSheet(labelTotal->styleSheet());
    globalForm->addRow("📌 总上架图书：", labelTotal);
    globalForm->addRow("✅ 已售图书：", labelSold);
    globalForm->addRow("⛔ 已下架图书：", labelOffShelf);
    globalForm->addRow("💰 总交易金额(元)：", labelTotalAmount);
    QPushButton *btnStats = new QPushButton("🔄 刷新全局统计");
    btnStats->setStyleSheet(SECONDARY_BUTTON_STYLE);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::onStatisticsClicked);
    globalForm->addRow(btnStats);
    statsLayout->addWidget(globalGroup);

    // 按时间段统计
    QGroupBox *timeGroup = new QGroupBox("📅 按时间段统计已售交易");
    timeGroup->setStyleSheet(queryGroup->styleSheet());
    QGridLayout *timeLayout = new QGridLayout(timeGroup);
    dateStart = new QDateEdit(); dateStart->setCalendarPopup(true);
    dateStart->setDate(QDate::currentDate().addMonths(-1));
    dateStart->setDisplayFormat("yyyy-MM-dd");
    dateEnd = new QDateEdit(); dateEnd->setCalendarPopup(true);
    dateEnd->setDate(QDate::currentDate());
    dateEnd->setDisplayFormat("yyyy-MM-dd");
    btnTimeStats = new QPushButton("📈 统计");
    btnTimeStats->setStyleSheet(PRIMARY_BUTTON_STYLE);
    labelTimeSold = new QLabel("0"); labelTimeAmount = new QLabel("0.00");
    labelCategory1 = new QLabel("0"); labelCategory2 = new QLabel("0");
    labelCategory3 = new QLabel("0"); labelCategory4 = new QLabel("0");
    timeLayout->addWidget(new QLabel("起始日期："),0,0); timeLayout->addWidget(dateStart,0,1);
    timeLayout->addWidget(new QLabel("结束日期："),0,2); timeLayout->addWidget(dateEnd,0,3);
    timeLayout->addWidget(btnTimeStats,0,4);
    timeLayout->addWidget(new QLabel("时间段内已售数量："),1,0,1,2); timeLayout->addWidget(labelTimeSold,1,2);
    timeLayout->addWidget(new QLabel("总交易金额："),1,3); timeLayout->addWidget(labelTimeAmount,1,4);
    timeLayout->addWidget(new QLabel("📘 教材成交量："),2,0); timeLayout->addWidget(labelCategory1,2,1);
    timeLayout->addWidget(new QLabel("📗 教辅成交量："),2,2); timeLayout->addWidget(labelCategory2,2,3);
    timeLayout->addWidget(new QLabel("📙 课外读物成交量："),3,0); timeLayout->addWidget(labelCategory3,3,1);
    timeLayout->addWidget(new QLabel("📕 其他成交量："),3,2); timeLayout->addWidget(labelCategory4,3,3);
    connect(btnTimeStats, &QPushButton::clicked, this, &MainWindow::onTimeStatisticsClicked);
    statsLayout->addWidget(timeGroup);

    // 导出报表和清空数据按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnExport = new QPushButton("💾 导出统计报表");
    btnExport->setStyleSheet(SECONDARY_BUTTON_STYLE);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportClicked);
    btnClearData = new QPushButton("⚠️ 清空所有测试数据（管理员）");
    btnClearData->setStyleSheet(DANGER_BUTTON_STYLE);
    connect(btnClearData, &QPushButton::clicked, this, &MainWindow::onClearDataClicked);
    buttonLayout->addWidget(btnExport);
    buttonLayout->addWidget(btnClearData);
    statsLayout->addLayout(buttonLayout);

    // 批量导入导出按钮（仅管理员可见）
    QHBoxLayout *batchLayout = new QHBoxLayout();
    btnBatchImport = new QPushButton("📂 批量导入CSV");
    btnBatchImport->setStyleSheet(PRIMARY_BUTTON_STYLE);
    connect(btnBatchImport, &QPushButton::clicked, this, &MainWindow::onBatchImport);
    btnBatchExport = new QPushButton("💾 批量导出CSV");
    btnBatchExport->setStyleSheet(PRIMARY_BUTTON_STYLE);
    connect(btnBatchExport, &QPushButton::clicked, this, &MainWindow::onBatchExport);
    batchLayout->addStretch();
    batchLayout->addWidget(btnBatchImport);
    batchLayout->addWidget(btnBatchExport);
    batchLayout->addStretch();
    statsLayout->addLayout(batchLayout);

    statsLayout->addStretch();
    stackedWidget->addWidget(pageStats);

    // -------------------- 页面5：交易记录（管理员） --------------------
    QWidget *pageTrade = new QWidget();
    QVBoxLayout *tradeLayout = new QVBoxLayout(pageTrade);
    tradeLayout->setContentsMargins(20,20,20,20);
    QPushButton *btnRefreshTrade = new QPushButton("刷新交易记录");
    btnRefreshTrade->setStyleSheet(SECONDARY_BUTTON_STYLE);
    connect(btnRefreshTrade, &QPushButton::clicked, this, &MainWindow::refreshTradeRecordTable);
    tradeLayout->addWidget(btnRefreshTrade);
    tableTradeRecords = new QTableWidget(this);
    tableTradeRecords->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableTradeRecords->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableTradeRecords->setAlternatingRowColors(true);
    tableTradeRecords->setStyleSheet(tableBooks->styleSheet());
    tradeLayout->addWidget(tableTradeRecords);
    stackedWidget->addWidget(pageTrade);

    // -------------------- 页面6：人员管理（管理员） --------------------
    QWidget *pageUser = new QWidget();
    QVBoxLayout *userLayout = new QVBoxLayout(pageUser);
    userLayout->setContentsMargins(20,20,20,20);
    userLayout->setSpacing(15);

    // 添加新用户分组
    QGroupBox *addGroup = new QGroupBox("添加新用户");
    addGroup->setStyleSheet(queryGroup->styleSheet());
    QHBoxLayout *addLayout = new QHBoxLayout(addGroup);
    editNewUsername = new QLineEdit(); editNewUsername->setPlaceholderText("新账号");
    editNewPassword = new QLineEdit(); editNewPassword->setPlaceholderText("密码");
    editNewPassword->setEchoMode(QLineEdit::Password);
    btnAddUser = new QPushButton("➕ 添加普通用户");
    btnAddUser->setStyleSheet(PRIMARY_BUTTON_STYLE);
    connect(btnAddUser, &QPushButton::clicked, this, &MainWindow::onAddUser);
    addLayout->addWidget(editNewUsername);
    addLayout->addWidget(editNewPassword);
    addLayout->addWidget(btnAddUser);
    addLayout->addStretch();
    userLayout->addWidget(addGroup);

    // 现有用户列表分组
    QGroupBox *listGroup = new QGroupBox("现有用户列表（含密码）");
    listGroup->setStyleSheet(queryGroup->styleSheet());
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    tableUsers = new QTableWidget(this);
    tableUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableUsers->setAlternatingRowColors(true);
    tableUsers->setStyleSheet(tableBooks->styleSheet());
    listLayout->addWidget(tableUsers);
    QHBoxLayout *delLayout = new QHBoxLayout();
    btnDeleteUser = new QPushButton("🗑️ 删除选中用户");
    btnDeleteUser->setStyleSheet(DANGER_BUTTON_STYLE);
    connect(btnDeleteUser, &QPushButton::clicked, this, &MainWindow::onDeleteUser);
    delLayout->addStretch();
    delLayout->addWidget(btnDeleteUser);
    listLayout->addLayout(delLayout);
    userLayout->addWidget(listGroup);

    // 额外操作按钮行：修改密码、切换管理员身份
    QHBoxLayout *actionLayout = new QHBoxLayout();
    btnChangePwd = new QPushButton("🔒 修改密码");
    btnChangePwd->setStyleSheet(PRIMARY_BUTTON_STYLE);
    connect(btnChangePwd, &QPushButton::clicked, this, &MainWindow::onChangePassword);
    btnToggleRole = new QPushButton("🔄 切换管理员身份");
    btnToggleRole->setStyleSheet(PRIMARY_BUTTON_STYLE);
    connect(btnToggleRole, &QPushButton::clicked, this, &MainWindow::onToggleRole);
    actionLayout->addStretch();
    actionLayout->addWidget(btnChangePwd);
    actionLayout->addWidget(btnToggleRole);
    actionLayout->addStretch();
    userLayout->addLayout(actionLayout);
    userLayout->addStretch();
    stackedWidget->addWidget(pageUser);

    // 组合左侧导航栏和右侧堆叠窗口
    horLayout->addWidget(navList);
    horLayout->addWidget(stackedWidget, 1);
    mainLayout->addLayout(horLayout);

    initUserTableHeader();      // 初始化用户表格表头
    navList->setCurrentRow(0);  // 默认选中图书浏览页
}

// 导航栏切换槽函数：切换右侧显示的页面
void MainWindow::onNavigationChanged(int index) {
    stackedWidget->setCurrentIndex(index);
    if (index == 3) refreshMyBooksTable();   // 切换到“我的图书”时刷新
    if (index == 6) refreshUserTable();      // 切换到“人员管理”时刷新
}

// ==================== 表格初始化函数 ====================
// 图书浏览表格表头（含ISBN）
void MainWindow::initTableHeader() {
    QStringList headers = {"编号", "ISBN", "书名", "作者", "出版社", "价格", "卖家", "状态", "发布时间"};
    tableBooks->setColumnCount(headers.size());
    tableBooks->setHorizontalHeaderLabels(headers);
    tableBooks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableBooks->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    tableBooks->setColumnWidth(1, 150);
    for (int i = 2; i < headers.size(); ++i) {
        tableBooks->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
}

// “我的图书”表格表头
void MainWindow::initMyBooksTableHeader() {
    QStringList headers = {"编号", "ISBN", "书名", "作者", "出版社", "价格", "状态"};
    myBooksTable->setColumnCount(headers.size());
    myBooksTable->setHorizontalHeaderLabels(headers);
    myBooksTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    myBooksTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    myBooksTable->setColumnWidth(1, 150);
    for (int i = 2; i < headers.size(); ++i) {
        myBooksTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
}

// 交易记录表格表头
void MainWindow::initTradeRecordTableHeader() {
    QStringList headers = {"图书编号", "书名", "卖家学号", "卖家姓名", "买家学号", "交易价格", "交易时间"};
    tableTradeRecords->setColumnCount(headers.size());
    tableTradeRecords->setHorizontalHeaderLabels(headers);
    tableTradeRecords->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// 用户管理表格表头（含密码）
void MainWindow::initUserTableHeader() {
    QStringList headers = {"账号", "密码", "角色"};
    tableUsers->setColumnCount(headers.size());
    tableUsers->setHorizontalHeaderLabels(headers);
    tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// ==================== 用户管理相关函数 ====================
// 刷新用户表格：重新加载CSV并显示
void MainWindow::refreshUserTable() {
    if (user_list != NULL) {
        free_user_list();
    }
    init_user_list();
    load_users_from_csv("users.csv");
    tableUsers->setRowCount(0);
    UserNode* p = user_list->next;
    int row = 0;
    while (p) {
        tableUsers->insertRow(row);
        tableUsers->setItem(row, 0, new QTableWidgetItem(p->data.username));
        tableUsers->setItem(row, 1, new QTableWidgetItem(p->data.password));
        QString roleStr = (p->data.role == 1) ? "管理员" : "普通用户";
        tableUsers->setItem(row, 2, new QTableWidgetItem(roleStr));
        row++;
        p = p->next;
    }
}

// 添加新用户
void MainWindow::onAddUser() {
    QString username = editNewUsername->text().trimmed();
    QString password = editNewPassword->text().trimmed();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "账号和密码不能为空");
        return;
    }
    if (user_list == NULL) {
        init_user_list();
        load_users_from_csv("users.csv");
    }
    int ret = add_user(username.toUtf8().data(), password.toUtf8().data(), 0);
    if (ret == -1) {
        QMessageBox::warning(this, "错误", "用户已存在");
    } else {
        QMessageBox::information(this, "成功", "添加成功");
        refreshUserTable();
        editNewUsername->clear();
        editNewPassword->clear();
    }
}

// 删除选中的用户（不能删除管理员和当前登录用户）
void MainWindow::onDeleteUser() {
    int row = tableUsers->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选中要删除的用户");
        return;
    }
    QString username = tableUsers->item(row, 0)->text();
    QString role = tableUsers->item(row, 2)->text();
    if (role == "管理员") {
        QMessageBox::warning(this, "警告", "不能删除管理员账号");
        return;
    }
    if (username == m_currentUserId) {
        QMessageBox::warning(this, "警告", "不能删除当前登录的账号");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
                                                              "确定要删除用户 " + username + " 吗？",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (user_list == NULL) {
        init_user_list();
        load_users_from_csv("users.csv");
    }
    int ret = delete_user(username.toUtf8().data());
    if (ret == 0) {
        QMessageBox::information(this, "成功", "删除成功");
        refreshUserTable();
    } else {
        QMessageBox::warning(this, "错误", "删除失败");
    }
}

// 修改用户密码（管理员操作）
void MainWindow::onChangePassword() {
    int row = tableUsers->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选中要修改密码的用户");
        return;
    }
    QString username = tableUsers->item(row, 0)->text();
    bool ok;
    QString newPwd = QInputDialog::getText(this, "修改密码",
                                           "请输入用户 " + username + " 的新密码：",
                                           QLineEdit::Password, "", &ok);
    if (!ok || newPwd.isEmpty()) return;

    if (user_list == NULL) {
        init_user_list();
        load_users_from_csv("users.csv");
    }
    int ret = update_user_password(username.toUtf8().data(), newPwd.toUtf8().data());
    if (ret == 0) {
        QMessageBox::information(this, "成功", "密码修改成功");
        refreshUserTable();
    } else {
        QMessageBox::warning(this, "错误", "修改密码失败，用户不存在");
    }
}

// 切换用户的管理员身份（普通用户↔管理员）
void MainWindow::onToggleRole() {
    int row = tableUsers->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选中要切换身份的用户");
        return;
    }
    QString username = tableUsers->item(row, 0)->text();
    QString currentRole = tableUsers->item(row, 2)->text();

    // 禁止修改 admin 账号的身份
    if (username == "admin") {
        QMessageBox::warning(this, "禁止操作", "admin 账号身份不可更改");
        return;
    }
    // 禁止修改当前登录账号的身份（防止将自己降级）
    if (username == m_currentUserId) {
        QMessageBox::warning(this, "禁止操作", "不能修改当前登录账号的身份");
        return;
    }

    int newRole = (currentRole == "管理员") ? 0 : 1;
    QString newRoleStr = (newRole == 1) ? "管理员" : "普通用户";
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认切换",
                                                              "将用户 " + username + " 的身份从 " + currentRole + " 改为 " + newRoleStr + "，确定吗？",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (user_list == NULL) {
        init_user_list();
        load_users_from_csv("users.csv");
    }
    int ret = update_user_role(username.toUtf8().data(), newRole);
    if (ret == 0) {
        QMessageBox::information(this, "成功", "身份切换成功");
        refreshUserTable();
    } else if (ret == -2) {
        QMessageBox::warning(this, "错误", "admin 账号身份不可更改");
    } else {
        QMessageBox::warning(this, "错误", "用户不存在或切换失败");
    }
}

// ==================== 图书显示通用函数 ====================
// 将链表中的图书数据显示到指定的表格中，isMyBooksPage 为 true 时表示“我的图书”页面（需要过滤卖家）
void MainWindow::displayBookList(LinkList list, QTableWidget *table, bool isMyBooksPage) {
    table->setRowCount(0);
    Node* p = list->next;
    int row = 0;
    while (p) {
        // 如果是“我的图书”页面且当前用户不是管理员，则只显示自己发布的图书
        if (isMyBooksPage && m_currentRole != ROLE_ADMIN && QString(p->data.seller_id) != m_currentUserId) {
            p = p->next;
            continue;
        }
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::number(p->data.book_id)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(p->data.isbn)));
        table->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(p->data.book_name)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromUtf8(p->data.author)));
        table->setItem(row, 4, new QTableWidgetItem(QString::fromUtf8(p->data.publisher)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(p->data.price, 'f', 2)));
        if (table == myBooksTable) {
            // 我的图书表格有状态列
            table->setItem(row, 6, new QTableWidgetItem(statusToString(p->data.status)));
            QColor color = (p->data.status == STATUS_ON_SALE) ? Qt::darkGreen : (p->data.status == STATUS_SOLD) ? Qt::darkRed : Qt::gray;
            table->item(row, 6)->setForeground(QBrush(color));
        } else {
            // 图书浏览表格有卖家、状态、发布时间
            table->setItem(row, 6, new QTableWidgetItem(QString::fromUtf8(p->data.seller_name)));
            table->setItem(row, 7, new QTableWidgetItem(statusToString(p->data.status)));
            table->setItem(row, 8, new QTableWidgetItem(QString::fromUtf8(p->data.publish_time)));
            QColor color = (p->data.status == STATUS_ON_SALE) ? Qt::darkGreen : (p->data.status == STATUS_SOLD) ? Qt::darkRed : Qt::gray;
            table->item(row, 7)->setForeground(QBrush(color));
        }
        row++; p = p->next;
    }
}

// 刷新图书浏览表格（显示所有图书）
void MainWindow::refreshBookTable() {
    freeCurrentDisplayList();
    displayBookList(book_list, tableBooks, false);
}

// 刷新“我的图书”表格（根据角色过滤）
void MainWindow::refreshMyBooksTable() {
    displayBookList(book_list, myBooksTable, true);
}

// 释放当前查询结果链表（避免内存泄漏）
void MainWindow::freeCurrentDisplayList() {
    if (m_currentDisplayList && m_currentDisplayList != book_list) {
        free_result_list(m_currentDisplayList);
    }
    m_currentDisplayList = nullptr;
}

// ==================== 图书业务函数 ====================
// 将状态枚举转换为中文字符串
QString MainWindow::statusToString(BookStatus status) {
    switch(status) {
    case STATUS_ON_SALE: return "在售";
    case STATUS_SOLD: return "已售";
    case STATUS_OFF_SHELF: return "已下架";
    default: return "未知";
    }
}

// 多条件查询图书
void MainWindow::onQueryBooks() {
    int index = comboQueryType->currentIndex();
    int type = 0;
    switch (index) {
    case 0: type = 1; break;  // 书名关键字
    case 1: type = 2; break;  // 作者
    case 2: type = 3; break;  // ISBN
    case 3: type = 6; break;  // 价格区间
    case 4: type = 4; break;  // 卖家学号
    case 5: type = 5; break;  // 交易状态
    default: type = 1;
    }
    QString keyword;
    float minPrice = 0, maxPrice = 999999;
    if (type == 6) {
        bool ok1, ok2;
        minPrice = editPriceMin->text().toFloat(&ok1);
        maxPrice = editPriceMax->text().toFloat(&ok2);
        if (!ok1) minPrice = 0;
        if (!ok2) maxPrice = 999999;
        if (minPrice > maxPrice) std::swap(minPrice, maxPrice);
        keyword = "";
    } else {
        keyword = editQueryKeyword->text().trimmed();
        if (keyword.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入查询关键字");
            return;
        }
        if (type == 5) {
            if (keyword == "在售") keyword = "0";
            else if (keyword == "已售") keyword = "1";
            else if (keyword == "已下架") keyword = "2";
            else {
                QMessageBox::warning(this, "警告", "状态请输入：在售/已售/已下架");
                return;
            }
        }
    }
    freeCurrentDisplayList();
    m_currentDisplayList = query_books_by_condition(type, keyword.toUtf8().data(), minPrice, maxPrice);
    if (m_currentDisplayList->next == nullptr)
        QMessageBox::information(this, "查询结果", "没有找到符合条件的图书。");
    displayBookList(m_currentDisplayList, tableBooks, false);
}

// 重置查询：显示所有图书
void MainWindow::onResetQuery() {
    refreshBookTable();
}

// 按书名查找图书（弹出选择对话框）
void MainWindow::onSearchByNameForBuy() {
    QString keyword = editBuyByName->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入书名关键字");
        return;
    }
    LinkList result = query_books_by_condition(1, keyword.toUtf8().data(), 0, 0);
    if (result->next == nullptr) {
        QMessageBox::information(this, "查找结果", "未找到匹配的图书");
        free_result_list(result);
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("选择要购买的图书");
    dialog.resize(750,450);
    QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
    QTableWidget *resultTable = new QTableWidget(&dialog);
    QStringList headers = {"编号","书名","价格","卖家","状态"};
    resultTable->setColumnCount(headers.size());
    resultTable->setHorizontalHeaderLabels(headers);
    Node* p = result->next;
    int row=0;
    while(p) {
        if (p->data.status == STATUS_ON_SALE) {
            resultTable->insertRow(row);
            resultTable->setItem(row,0, new QTableWidgetItem(QString::number(p->data.book_id)));
            resultTable->setItem(row,1, new QTableWidgetItem(QString::fromUtf8(p->data.book_name)));
            resultTable->setItem(row,2, new QTableWidgetItem(QString::number(p->data.price)));
            resultTable->setItem(row,3, new QTableWidgetItem(QString::fromUtf8(p->data.seller_name)));
            resultTable->setItem(row,4, new QTableWidgetItem(statusToString(p->data.status)));
            row++;
        }
        p = p->next;
    }
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dlgLayout->addWidget(resultTable);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dlgLayout->addWidget(buttonBox);
    if (dialog.exec() == QDialog::Accepted) {
        int curRow = resultTable->currentRow();
        if (curRow >= 0) {
            int bookId = resultTable->item(curRow,0)->text().toInt();
            editBuyId->setText(QString::number(bookId));
            QMessageBox::information(this, "提示", "已自动填入图书编号，请点击\"确认购买\"完成交易。");
        } else {
            QMessageBox::warning(this, "提示", "请先选中一本图书");
        }
    }
    free_result_list(result);
}

// 发布图书
void MainWindow::onPublishClicked() {
    Book book;
    strcpy(book.book_name, editBookName->text().toUtf8().data());
    strcpy(book.isbn, editIsbn->text().toUtf8().data());
    strcpy(book.author, editAuthor->text().toUtf8().data());
    strcpy(book.publisher, editPublisher->text().toUtf8().data());
    book.price = editPrice->text().toFloat();
    strcpy(book.seller_id, m_currentUserId.toUtf8().data());
    strcpy(book.seller_name, editSellerName->text().toUtf8().data());
    strcpy(book.seller_phone, editPhone->text().toUtf8().data());
    book.category = comboCategory->currentIndex() + 1;
    if (strlen(book.book_name)==0 || book.price<=0) {
        QMessageBox::warning(this, "警告", "书名和价格不能为空");
        return;
    }
    int ret = add_book(book);
    if (ret == 0) {
        QMessageBox::information(this, "成功", "图书发布成功！编号：" + QString::number(next_book_id-1));
        refreshBookTable();
        refreshMyBooksTable();
        editBookName->clear(); editIsbn->clear(); editAuthor->clear(); editPublisher->clear();
        editPrice->clear(); editSellerName->clear(); editPhone->clear();
    } else {
        QMessageBox::critical(this, "错误", "发布失败！");
    }
}

// 购买图书
void MainWindow::onBuyClicked() {
    int bookId = editBuyId->text().toInt();
    if (bookId == 0) {
        QMessageBox::warning(this, "警告", "请输入有效的图书编号");
        return;
    }
    Node* book = query_book_by_id(bookId);
    if (!book) {
        QMessageBox::critical(this, "错误", "图书不存在");
        return;
    }
    // 普通用户不能购买自己发布的图书
    if (m_currentRole == ROLE_USER && strcmp(book->data.seller_id, m_currentUserId.toUtf8().data()) == 0) {
        QMessageBox::warning(this, "提示", "不能购买自己发布的图书");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认购买",
                                                              "确认购买《"+QString::fromUtf8(book->data.book_name)+"》？\n价格："+QString::number(book->data.price)+"元",
                                                              QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    int ret = sell_book(bookId, m_currentUserId.toUtf8().data());
    if (ret == 0) {
        QMessageBox::information(this, "成功", "购买成功！");
        refreshBookTable();
        refreshMyBooksTable();
        editBuyId->clear();
    } else if (ret == -2) {
        QMessageBox::critical(this, "错误", "该图书已售或已下架！");
    }
}

// 下架图书（从“我的图书”页面选中）
void MainWindow::onOffShelfClicked() {
    int currentRow = myBooksTable->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "警告", "请先在\"我的图书\"页选中要下架的图书");
        return;
    }
    int bookId = myBooksTable->item(currentRow, 0)->text().toInt();
    Node* book = query_book_by_id(bookId);
    if (!book) return;
    if (m_currentRole != ROLE_ADMIN && strcmp(book->data.seller_id, m_currentUserId.toUtf8().data()) != 0) {
        QMessageBox::critical(this, "权限不足", "您只能下架自己发布的图书");
        return;
    }
    int ret = off_shelf_book(bookId, book->data.seller_id);
    if (ret == 0) {
        QMessageBox::information(this, "成功", "图书下架成功");
        refreshBookTable();
        refreshMyBooksTable();
    } else if (ret == -3) {
        QMessageBox::critical(this, "错误", "该图书已售或已下架");
    }
}

// 管理员删除图书
void MainWindow::onDeleteBookClicked() {
    int currentRow = tableBooks->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "警告", "请先选中要删除的图书");
        return;
    }
    int bookId = tableBooks->item(currentRow,0)->text().toInt();
    QString bookName = tableBooks->item(currentRow,2)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
                                                              "确定要永久删除《"+bookName+"》吗？\n此操作不可恢复！",
                                                              QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    int ret = delete_book(bookId);
    if (ret == 0) {
        QMessageBox::information(this, "成功", "图书删除成功");
        refreshBookTable();
        refreshMyBooksTable();
    } else {
        QMessageBox::critical(this, "错误", "删除失败");
    }
}

// 全局统计刷新
void MainWindow::onStatisticsClicked() {
    int total,sold,offShelf; float totalAmount;
    get_statistics(&total,&sold,&offShelf,&totalAmount);
    labelTotal->setText(QString::number(total));
    labelSold->setText(QString::number(sold));
    labelOffShelf->setText(QString::number(offShelf));
    labelTotalAmount->setText(QString::number(totalAmount,'f',2));
}

// 按时间段统计
void MainWindow::onTimeStatisticsClicked() {
    QString start = dateStart->date().toString("yyyy-MM-dd")+" 00:00";
    QString end = dateEnd->date().toString("yyyy-MM-dd")+" 23:59";
    int soldCount; float amount; int catSales[5]={0};
    get_statistics_by_time(start.toUtf8().data(), end.toUtf8().data(), &soldCount, &amount, catSales);
    labelTimeSold->setText(QString::number(soldCount));
    labelTimeAmount->setText(QString::number(amount,'f',2));
    labelCategory1->setText(QString::number(catSales[1]));
    labelCategory2->setText(QString::number(catSales[2]));
    labelCategory3->setText(QString::number(catSales[3]));
    labelCategory4->setText(QString::number(catSales[4]));
}

// 导出统计报表到文本文件
void MainWindow::onExportClicked() {
    int total,sold,offShelf; float totalAmount;
    get_statistics(&total,&sold,&offShelf,&totalAmount);
    QString fileName = QFileDialog::getSaveFileName(this,"保存统计报表","","文本文件 (*.txt)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法保存文件");
        return;
    }
    QTextStream out(&file);
    out << "==============================\n";
    out << "  校园二手书交易系统统计报表\n";
    out << "==============================\n";
    out << "统计时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";
    out << "【全局统计】\n";
    out << "总上架图书：" << total << " 本\n";
    out << "已售图书：" << sold << " 本\n";
    out << "已下架图书：" << offShelf << " 本\n";
    out << "总交易金额：" << QString::number(totalAmount,'f',2) << " 元\n\n";
    out << "【近一个月交易统计】\n";
    QDate now = QDate::currentDate();
    QDate startDate = now.addMonths(-1);
    QString start = startDate.toString("yyyy-MM-dd")+" 00:00";
    QString end = now.toString("yyyy-MM-dd")+" 23:59";
    int soldCount; float monthAmount; int catSales[5];
    get_statistics_by_time(start.toUtf8().data(), end.toUtf8().data(), &soldCount, &monthAmount, catSales);
    out << "时间段：" << startDate.toString("yyyy-MM-dd") << " 至 " << now.toString("yyyy-MM-dd") << "\n";
    out << "成交量：" << soldCount << " 本\n";
    out << "交易总额：" << QString::number(monthAmount,'f',2) << " 元\n";
    out << "教材成交量：" << catSales[1] << "\n";
    out << "教辅成交量：" << catSales[2] << "\n";
    out << "课外读物成交量：" << catSales[3] << "\n";
    out << "其他成交量：" << catSales[4] << "\n";
    out << "==============================\n";
    file.close();
    QMessageBox::information(this, "成功", "统计报表已导出到：\n"+fileName);
}

// 清空所有图书数据（仅管理员）
void MainWindow::onClearDataClicked() {
    if (m_currentRole != ROLE_ADMIN) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以清空所有数据！");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认清空", "确定要清空所有图书数据吗？\n此操作不可恢复！", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    free_list(); init_list(); next_book_id=10001; save_to_csv("books.csv");
    refreshBookTable();
    refreshMyBooksTable();
    QMessageBox::information(this, "成功", "所有数据已清空！");
}

// 批量导入CSV文件（追加方式）
void MainWindow::onBatchImport() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择CSV文件", "", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        return;
    }
    QTextStream in(&file);
    QString line = in.readLine(); // 跳过标题行
    int added = 0, failed = 0;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList fields = line.split(',');
        if (fields.size() < 14) continue;
        Book book;
        book.book_id = fields[0].toInt();
        strcpy(book.isbn, fields[1].toUtf8().data());
        strcpy(book.book_name, fields[2].toUtf8().data());
        strcpy(book.author, fields[3].toUtf8().data());
        strcpy(book.publisher, fields[4].toUtf8().data());
        book.price = fields[5].toFloat();
        strcpy(book.seller_id, fields[6].toUtf8().data());
        strcpy(book.seller_name, fields[7].toUtf8().data());
        strcpy(book.seller_phone, fields[8].toUtf8().data());
        book.status = (BookStatus)fields[9].toInt();
        strcpy(book.publish_time, fields[10].toUtf8().data());
        strcpy(book.buyer_id, fields[11].toUtf8().data());
        strcpy(book.trade_time, fields[12].toUtf8().data());
        book.category = fields[13].toInt();
        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->data = book;
        new_node->next = book_list->next;
        book_list->next = new_node;
        if (book.book_id >= next_book_id) next_book_id = book.book_id + 1;
        added++;
    }
    file.close();
    save_to_csv("books.csv");
    refreshBookTable();
    refreshMyBooksTable();
    QMessageBox::information(this, "导入完成", QString("成功导入 %1 本图书，失败 %2 本").arg(added).arg(failed));
}

// 批量导出所有图书到CSV文件
void MainWindow::onBatchExport() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存CSV文件", "", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    save_to_csv(fileName.toUtf8().data());
    QMessageBox::information(this, "导出完成", "图书数据已导出到：" + fileName);
}

// 刷新交易记录表格
void MainWindow::refreshTradeRecordTable() {
    tableTradeRecords->setRowCount(0);
    Node* records = get_all_trade_records();
    Node* p = records->next;
    int row = 0;
    while (p) {
        tableTradeRecords->insertRow(row);
        tableTradeRecords->setItem(row,0, new QTableWidgetItem(QString::number(p->data.book_id)));
        tableTradeRecords->setItem(row,1, new QTableWidgetItem(QString::fromUtf8(p->data.book_name)));
        tableTradeRecords->setItem(row,2, new QTableWidgetItem(QString::fromUtf8(p->data.seller_id)));
        tableTradeRecords->setItem(row,3, new QTableWidgetItem(QString::fromUtf8(p->data.seller_name)));
        tableTradeRecords->setItem(row,4, new QTableWidgetItem(QString::fromUtf8(p->data.buyer_id)));
        tableTradeRecords->setItem(row,5, new QTableWidgetItem(QString::number(p->data.price,'f',2)));
        tableTradeRecords->setItem(row,6, new QTableWidgetItem(QString::fromUtf8(p->data.trade_time)));
        row++; p = p->next;
    }
    free_result_list(records);
}