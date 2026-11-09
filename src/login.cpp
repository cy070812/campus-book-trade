#include "login.h"
#include "mainwindow.h"
#include "user_core.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

/**
 * @brief Login构造函数，初始化登录窗体
 * @param parent 父窗口指针
 */
Login::Login(QWidget *parent) : QMainWindow(parent), m_mainWindow(nullptr) {
    setupUI();
}

/**
 * @brief 登录窗口析构函数，释放主界面窗口内存，防止内存泄漏
 */
Login::~Login() {
    delete m_mainWindow;
}

/**
 * @brief setupUI 登录页面UI初始化函数，搭建登录界面所有控件与布局
 */
void Login::setupUI() {
    setWindowTitle("校园二手书交易系统");
    setFixedSize(450, 400);
    setStyleSheet("background-color: #fafafa;");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(50, 50, 50, 40);
    mainLayout->setSpacing(25);

    // 页面顶部标题标签
    QLabel* title = new QLabel("欢迎登录", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #1a1a1a;");
    mainLayout->addWidget(title);

    // 账号输入框控件，设置占位提示文字与输入框样式
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("账号");
    m_usernameEdit->setStyleSheet(
        "border: 1px solid #cccccc;"
        "border-radius: 12px;"
        "padding: 12px 16px;"
        "font-size: 14px;"
        );
    mainLayout->addWidget(m_usernameEdit);

    // 密码输入框，隐藏明文为密码样式，复用输入框样式
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet(m_usernameEdit->styleSheet());
    mainLayout->addWidget(m_passwordEdit);

    // 登录按钮，配置样式，绑定点击跳转登录逻辑槽函数
    m_loginBtn = new QPushButton("登录", this);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #1a1a1a;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 25px;"
        "   padding: 12px;"
        "   font-size: 16px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #333333; }"
        );
    connect(m_loginBtn, &QPushButton::clicked, this, &Login::onLogin);
    mainLayout->addWidget(m_loginBtn);

    // 注册按钮，配置边框样式，绑定注册事件槽函数
    m_registerBtn = new QPushButton("注册新账号", this);
    m_registerBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #ffffff;"
        "   color: #1a1a1a;"
        "   border: 1px solid #cccccc;"
        "   border-radius: 25px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #f0f0f0; }"
        );
    connect(m_registerBtn, &QPushButton::clicked, this, &Login::onRegister);
    mainLayout->addWidget(m_registerBtn);

    // 弹性占位，自适应布局留白
    mainLayout->addStretch();
}

/**
 * @brief onLogin 登录按钮触发槽函数，实现账号密码校验、权限判断、页面跳转
 */
void Login::onLogin() {
    // 获取输入框账号、密码，去除首尾空格
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    // 非空校验，账号或密码为空弹窗提示
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入账号和密码");
        return;
    }

    // 初始化用户链表，从users.csv文件加载全部用户数据
    init_user_list();
    load_users_from_csv("users.csv");
    // 根据用户名查找用户
    User* user = find_user(username.toUtf8().data());
    // 用户不存在弹窗提示，释放链表内存后结束
    if (!user) {
        QMessageBox::warning(this, "错误", "账号不存在，请先注册");
        free_user_list();
        return;
    }
    // 密码比对错误弹窗提示，释放内存
    if (strcmp(user->password, password.toUtf8().data()) != 0) {
        QMessageBox::warning(this, "错误", "密码错误");
        free_user_list();
        return;
    }

    // 根据role字段区分管理员/普通用户权限
    UserRole role = (user->role == 1) ? ROLE_ADMIN : ROLE_USER;
    // 释放用户链表堆内存
    free_user_list();

    // 隐藏登录窗口，创建系统主界面并传入当前登录用户与权限
    this->hide();
    m_mainWindow = new MainWindow();
    m_mainWindow->setCurrentUser(username, role);
    m_mainWindow->show();
}

/**
 * @brief onRegister 注册按钮槽函数，完成新用户注册逻辑，默认注册为普通用户
 */
void Login::onRegister() {
    // 获取账号密码并去除空格
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();
    // 账号密码空值校验
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "账号和密码不能为空");
        return;
    }

    // 初始化链表并加载本地用户数据
    init_user_list();
    load_users_from_csv("users.csv");
    // 查重：用户名已存在则提示，释放内存退出
    if (find_user(username.toUtf8().data())) {
        QMessageBox::warning(this, "错误", "账号已存在");
        free_user_list();
        return;
    }
    // 添加新用户，0代表普通用户权限
    int ret = add_user(username.toUtf8().data(), password.toUtf8().data(), 0);
    // 释放用户链表内存
    free_user_list();
    // 根据返回值提示注册结果，成功清空密码输入框
    if (ret == 0) {
        QMessageBox::information(this, "成功", "注册成功，请登录");
        m_passwordEdit->clear();
    } else {
        QMessageBox::critical(this, "错误", "注册失败");
    }
}