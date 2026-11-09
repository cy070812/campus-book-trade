#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include "common.h"

class MainWindow;

class Login : public QMainWindow {
    Q_OBJECT
public:
    Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void onLogin();
    void onRegister();

private:
    void setupUI();

    MainWindow* m_mainWindow;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginBtn;
    QPushButton* m_registerBtn;
};

#endif // LOGIN_H