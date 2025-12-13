#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include "NetworkManager.h"
#include "Validator.h"
#include "CTitleBar.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QFrame>



class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginRequested(const QString &ip, int port,
                       const QString &username, const QString &password);
    void registerRequested(const QString &ip, int port,
                          const QString &username, const QString &password);
    void connectionTestRequested(const QString &ip, int port);

    void sigLoginSuccess();
// 将public slots改为private slots，以匹配.cpp文件中的实现
private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onTestConnectionClicked();
    void onRememberMeToggled(bool checked);
    void onAutoLoginToggled(bool checked);

private:
    // 将这些函数从public slots移到private部分
    void onLoginResult(bool success, const QString &message);
    void onRegisterResult(bool success, const QString &message);
    void onConnectionTestResult(bool success, const QString &message);

    void setupUI();
    void setupConnections();
    void showMessage(const QString &message, bool isError = false);
    void setLoading(bool loading);
    QFrame* createLine();

    // UI 组件
    QLabel *titleLabel;
    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *testConnectionButton;
    QCheckBox *rememberMeCheck;
    QCheckBox *autoLoginCheck;
    QLabel *statusLabel;

    // 布局
    QVBoxLayout *loginLayout;
    QFormLayout *formLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *optionsLayout;

    // 功能类
    NetworkManager *networkManager;
    bool isLoading;
    CTitleBar* _ptitleBar;      // 标题栏
};

#endif // LOGINWIDGET_H
