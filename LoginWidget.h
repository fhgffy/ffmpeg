#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "cframelesswidgetbase.h" // 【修改】引入无边框基类
#include "NetworkManager.h"
#include "Validator.h"
#include "CTitleBar.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QCloseEvent> // 引入關閉事件
// 继承 CFrameLessWidgetBase
class LoginWidget : public CFrameLessWidgetBase
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
    void sigLoginSuccess();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onRememberMeToggled(bool checked);
    void onAutoLoginToggled(bool checked);
    void onShowSettingDialog(); // 【新增】显示设置弹窗
protected:
    // 【新增】重寫關閉事件，用於在點擊“×”時保存設置
    void closeEvent(QCloseEvent *event) override;
private:
    void onLoginResult(bool success, const QString &message);
    void onRegisterResult(bool success, const QString &message);

    void setupUI();
    void setupConnections();
    void showMessage(const QString &message, bool isError = false);
    void setLoading(bool loading);

    // 加載和保存設置的輔助函數
        void loadSavedSettings();
        void saveLoginSettings();
    // UI 组件
    CTitleBar* m_titleBar;    // 【新增】自定义标题栏
    QLabel *m_logoLabel;      // 【新增】Logo显示
    QLabel *m_welcomeLabel;   // 【新增】欢迎标语

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;

    QPushButton *loginButton;
    QPushButton *registerButton;

    QCheckBox *rememberMeCheck;
    QCheckBox *autoLoginCheck;
    QLabel *statusLabel;
    // 【新增】自動登錄定時器，方便隨時停止
    QTimer *m_autoLoginTimer;
    int m_autoLoginCount; // 【新增】用於記錄倒計時剩餘秒數
    // 功能类
    NetworkManager *networkManager;
    bool isLoading;
};

#endif // LOGINWIDGET_H
