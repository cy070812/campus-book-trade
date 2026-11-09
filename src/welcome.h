#ifndef WELCOME_H
#define WELCOME_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QWidget>

class Login;

class Welcome : public QMainWindow {
    Q_OBJECT

public:
    Welcome(QWidget *parent = nullptr);
    ~Welcome() override;

private slots:
    void onEnterButtonClicked();

private:
    void initUserInterface();
    Login *m_loginWindow;
};

#endif // WELCOME_H