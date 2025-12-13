

#include "LoginWidget.h"
#include "StyleHelper.h"
#include <QTimer>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QSettings> // 引入设置
#include <QDialog>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
LoginWidget::LoginWidget(QWidget *parent)
    : CFrameLessWidgetBase(parent)
    , networkManager(new NetworkManager(this))
    , isLoading(false)
    , m_autoLoginCount(0) // 初始化
{
    // 初始化自動登錄定時器
        m_autoLoginTimer = new QTimer(this);
        // 【注意】這裡去掉了 setSingleShot(true)，因爲我們要它每秒跳一次

        // 【核心邏輯修改】每秒執行一次
        connect(m_autoLoginTimer, &QTimer::timeout, this, [this](){
            m_autoLoginCount--; // 倒計時減一

            if (m_autoLoginCount > 0) {
                // 還沒到時間，更新按鈕文字
                loginButton->setText(QString("自動登錄中...(%1秒)").arg(m_autoLoginCount));
            } else {
                // 時間到，停止定時器並登錄
                m_autoLoginTimer->stop();
                onLoginClicked();
            }
        });
    setupUI();
    setupConnections();
    resize(430, 580);
    // 【新增】程序啓動時加載保存的設置
        loadSavedSettings();
}

LoginWidget::~LoginWidget()
{
}

void LoginWidget::setupUI()
{
    this->setStyleSheet("LoginWidget { background-color: #2b2b2b; border: 1px solid #454545; border-radius: 5px; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_titleBar = new CTitleBar(this);
    m_titleBar->setStyleSheet("background-color: transparent;");
    connect(m_titleBar, &CTitleBar::sigClose, this, &LoginWidget::close);
    mainLayout->addWidget(m_titleBar);

    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 10, 40, 40);
    contentLayout->setSpacing(20);

    m_logoLabel = new QLabel(contentWidget);
    m_logoLabel->setFixedSize(100, 100);
    QPixmap logoPix(":/logo/logo.png");
    m_logoLabel->setPixmap(logoPix);
    m_logoLabel->setScaledContents(true);
    m_logoLabel->setAlignment(Qt::AlignCenter);

    m_welcomeLabel = new QLabel("欢迎登录监控系统", contentWidget);
    m_welcomeLabel->setStyleSheet("color: #ffffff; font-size: 24px; font-weight: bold; font-family: 'Microsoft YaHei';");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->addStretch();
    logoLayout->addWidget(m_logoLabel);
    logoLayout->addStretch();
    contentLayout->addLayout(logoLayout);
    contentLayout->addWidget(m_welcomeLabel);
    contentLayout->addSpacing(10);

    QString inputStyle = R"(
        QLineEdit {
            background-color: #3e3e3e;
            border: 1px solid #505050;
            border-radius: 4px;
            color: #ffffff;
            padding: 10px 10px;
            font-size: 14px;
            selection-background-color: #0078d7;
        }
        QLineEdit:focus {
            border: 1px solid #0078d7;
            background-color: #454545;
        }
        QLineEdit:disabled {
            background-color: #333333;
            color: #888888;
        }
    )";

    usernameEdit = new QLineEdit(contentWidget);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setStyleSheet(inputStyle);
    usernameEdit->setMinimumHeight(45);

    passwordEdit = new QLineEdit(contentWidget);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(inputStyle);
    passwordEdit->setMinimumHeight(45);

    contentLayout->addWidget(usernameEdit);
    contentLayout->addWidget(passwordEdit);

    QHBoxLayout *optionsLayout = new QHBoxLayout();
    QString checkStyle = "QCheckBox { color: #cccccc; font-size: 12px; } "
                         "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; border: 1px solid #666; background: #333; }"
                         "QCheckBox::indicator:checked { background-color: #0078d7; border-color: #0078d7; image: url(:/resources/titlebar/normal.svg); }";

    rememberMeCheck = new QCheckBox("记住密码", contentWidget);
    rememberMeCheck->setStyleSheet(checkStyle);

    autoLoginCheck = new QCheckBox("自动登录", contentWidget);
    autoLoginCheck->setStyleSheet(checkStyle);

    optionsLayout->addWidget(rememberMeCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(autoLoginCheck);
    contentLayout->addLayout(optionsLayout);

    contentLayout->addSpacing(10);

    QString loginBtnStyle = R"(
        QPushButton {
            background-color: #0078d7;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 16px;
            font-weight: bold;
            padding: 12px;
        }
        QPushButton:hover { background-color: #198ae0; }
        QPushButton:pressed { background-color: #0063b1; }
        QPushButton:disabled { background-color: #444444; color: #888888; }
    )";

    QString regBtnStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #0078d7;
            border: 1px solid #0078d7;
            border-radius: 4px;
            font-size: 14px;
            padding: 10px;
        }
        QPushButton:hover { background-color: rgba(0, 120, 215, 0.1); }
        QPushButton:pressed { background-color: rgba(0, 120, 215, 0.2); }
        QPushButton:disabled { border-color: #555; color: #555; }
    )";

    loginButton = new QPushButton("登  录", contentWidget);
    loginButton->setStyleSheet(loginBtnStyle);
    loginButton->setCursor(Qt::PointingHandCursor);

    registerButton = new QPushButton("注册账号", contentWidget);
    registerButton->setStyleSheet(regBtnStyle);
    registerButton->setCursor(Qt::PointingHandCursor);

    contentLayout->addWidget(loginButton);
    contentLayout->addWidget(registerButton);

    statusLabel = new QLabel("", contentWidget);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #ff4d4f; font-size: 12px; min-height: 20px;");
    contentLayout->addWidget(statusLabel);

    mainLayout->addWidget(contentWidget);
    usernameEdit->setFocus();
}

void LoginWidget::setupConnections()
{
    connect(loginButton, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);

    connect(networkManager, &NetworkManager::loginSuccess, this, [this]() {
        onLoginResult(true, "登录成功，正在跳转...");
    });
    connect(networkManager, &NetworkManager::loginFailed, this, [this](const QString &error) {
        onLoginResult(false, error);
    });
    connect(networkManager, &NetworkManager::registrationSuccess, this, [this]() {
        onRegisterResult(true, "注册成功，请直接登录");
    });
    connect(networkManager, &NetworkManager::registrationFailed, this, [this](const QString &error) {
        onRegisterResult(false, error);
    });
    // 【新增】连接标题栏的设置信号
    connect(m_titleBar, &CTitleBar::sigSet, this, &LoginWidget::onShowSettingDialog);
    // 連接複選框的槽函數，處理邏輯聯動
    connect(rememberMeCheck, &QCheckBox::toggled, this, &LoginWidget::onRememberMeToggled);
    connect(autoLoginCheck, &QCheckBox::toggled, this, &LoginWidget::onAutoLoginToggled);
}

void LoginWidget::onLoginClicked()
{
    if (isLoading) return;
    statusLabel->clear();

    QSettings settings("config.ini", QSettings::IniFormat);
    // 读出来的 port 已经是 int
    QString ip = settings.value("Server/IP", "192.168.216.201").toString();
    int port = settings.value("Server/Port", 8888).toInt();

    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        showMessage("用户名或密码不能为空", true);
        return;
    }

    setLoading(true);
    showMessage("正在连接服务器...", false);

    // 【修正】去掉 .toInt()，直接传递 int 类型的 port
    emit loginRequested(ip, port, username, password);
    networkManager->login(ip, port, username, password,
                         [this](bool success, const QString &message) {
        QMetaObject::invokeMethod(this, [this, success, message]() {
            if (!success) {
                setLoading(false);
                showMessage(message, true);
            }
        });
    });
}

void LoginWidget::onRegisterClicked()
{
    if (isLoading) return;
    statusLabel->clear();

    QSettings settings("config.ini", QSettings::IniFormat);
    QString ip = settings.value("Server/IP", "192.168.216.201").toString();
    int port = settings.value("Server/Port", 8888).toInt();

    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        showMessage("请填写用户名和密码", true);
        return;
    }

    setLoading(true);
    showMessage("正在注册...", false);

    // 【修正】去掉 .toInt()，直接传递 int 类型的 port
    emit registerRequested(ip, port, username, password);
    networkManager->registerUser(ip, port, username, password,
                                [this](bool success, const QString &message) {
        QMetaObject::invokeMethod(this, [this, success, message]() {
             if (!success) {
                setLoading(false);
                showMessage(message, true);
            }
        });
    });
}

void LoginWidget::onLoginResult(bool success, const QString &message)
{
    if (success) {
        showMessage(message, false);
        loginButton->setText("登录成功");
        loginButton->setStyleSheet("QPushButton { background-color: #28a745; color: white; border-radius: 4px; font-weight: bold; padding: 12px; border: none;}");
        // 【新增】登錄成功，保存賬號密碼和狀態
        saveLoginSettings();
        QTimer::singleShot(500, this, [this](){
            emit sigLoginSuccess();
            this->close();
        });
    } else {
        setLoading(false);
        showMessage(message, true);
        // 登录失败，取消自动登录
        if (autoLoginCheck->isChecked()) {
                // 这会触发 onAutoLoginToggled(false)，从而停止定时器并恢复按钮文字
                autoLoginCheck->setChecked(false);
                statusLabel->setText("自动登录失败，已取消自动登录");
        }
    }
}

void LoginWidget::onRegisterResult(bool success, const QString &message)
{
    setLoading(false);
    if (success) {
        showMessage(message, false);
        passwordEdit->setFocus();
    } else {
        showMessage(message, true);
    }
}

void LoginWidget::showMessage(const QString &message, bool isError)
{
    statusLabel->setText(message);
    if (isError) {
        statusLabel->setStyleSheet("color: #ff4d4f; font-size: 13px;");
    } else {
        statusLabel->setStyleSheet("color: #28a745; font-size: 13px;");
    }
}

void LoginWidget::setLoading(bool loading)
{
    isLoading = loading;
    usernameEdit->setEnabled(!loading);
    passwordEdit->setEnabled(!loading);
    loginButton->setEnabled(!loading);
    registerButton->setEnabled(!loading);

    if (loading) {
        loginButton->setText("请稍候...");
        this->setCursor(Qt::WaitCursor);
    } else {
        loginButton->setText("登  录");
        this->setCursor(Qt::ArrowCursor);
    }
}

// 實現複選框聯動邏輯
void LoginWidget::onRememberMeToggled(bool checked)
{
    // 如果取消了「記住密碼」，則必須取消「自動登錄」
    if (!checked) {
        autoLoginCheck->setChecked(false);
    }
}
// 自動登錄勾選邏輯
void LoginWidget::onAutoLoginToggled(bool checked)
{
    if (checked) {
        // 勾选自动登录 -> 必须勾选记住密码
        rememberMeCheck->setChecked(true);

        // 【新增逻辑】只要勾选，且有账号密码，就启动 5 秒倒计时
        if (!usernameEdit->text().isEmpty() && !passwordEdit->text().isEmpty()) {
            // 如果定时器已经在跑（比如程序刚启动），就不重置了；
            // 如果是用户手动重新勾选（定时器已停），则重新开始。
            if (!m_autoLoginTimer->isActive()) {
                m_autoLoginCount = 5;
                loginButton->setText(QString("自动登录中...(%1秒)").arg(m_autoLoginCount));
                m_autoLoginTimer->start(1000);
            }
        }
    } else {
        // 取消勾选 -> 立即停止倒计时
        if (m_autoLoginTimer->isActive()) {
            m_autoLoginTimer->stop();
            loginButton->setText("登  录"); // 恢复按钮文字
        }
    }
}

// 2. 实现弹窗逻辑
void LoginWidget::onShowSettingDialog()
{
    // 创建一个简单的模态对话框
    QDialog dlg(this);
    dlg.setWindowTitle("服务器配置");
    dlg.setFixedSize(300, 150);
    dlg.setStyleSheet("QDialog { background-color: #3e3e3e; color: white; } "
                      "QLabel { color: white; } "
                      "QLineEdit, QSpinBox { background-color: #2b2b2b; color: white; border: 1px solid #505050; padding: 4px; }");

    QFormLayout *layout = new QFormLayout(&dlg);

    // 读取当前配置
    QSettings settings("config.ini", QSettings::IniFormat);
    QString currentIp = settings.value("Server/IP", "192.168.216.201").toString();
    int currentPort = settings.value("Server/Port", 8888).toInt();

    // IP 输入框
    QLineEdit *ipEdit = new QLineEdit(&dlg);
    ipEdit->setText(currentIp);

    // 端口输入框
    QSpinBox *portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1, 65535);
    portSpin->setValue(currentPort);
    portSpin->setButtonSymbols(QAbstractSpinBox::NoButtons); // 不显示上下箭头

    layout->addRow("服务器 IP:", ipEdit);
    layout->addRow("服务器端口:", portSpin);

    // 确认取消按钮
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btnBox->setStyleSheet("QPushButton { background-color: #0078d7; color: white; border: none; padding: 6px 12px; border-radius: 4px; } "
                          "QPushButton:hover { background-color: #198ae0; }");
    layout->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // 显示弹窗
    if (dlg.exec() == QDialog::Accepted) {
        // 保存配置
        settings.setValue("Server/IP", ipEdit->text().trimmed());
        settings.setValue("Server/Port", portSpin->value());
        settings.sync(); // 强制写入

        // 可选：提示用户
        statusLabel->setText("配置已更新，下次登录生效");
    }
}

void LoginWidget::loadSavedSettings()
{
    QSettings settings("config.ini", QSettings::IniFormat);

    bool remember = settings.value("Login/RememberMe", false).toBool();
    bool autoLogin = settings.value("Login/AutoLogin", false).toBool();

    // 【修改点1】先不要setChecked，先填数据
    // rememberMeCheck->setChecked(remember);
    // autoLoginCheck->setChecked(autoLogin);

    if (remember) {
        usernameEdit->setText(settings.value("Login/Username").toString());
        QByteArray passData = settings.value("Login/Password").toByteArray();
        passwordEdit->setText(QByteArray::fromBase64(passData));
    }

    // 【修改点2】数据填好后，最后再设置勾选状态
    // setChecked 会触发 onRememberMeToggled 和 onAutoLoginToggled 信号
    // 从而自动触发我们在 onAutoLoginToggled 里写的倒计时逻辑
    rememberMeCheck->setChecked(remember);

    // 这里为了防止逻辑冲突（比如先setChecked(true)触发了倒计时，但可能还没准备好），
    // 建议加个判断：只有当 autoLogin 为 true 时才去 setChecked
    if (autoLogin) {
        autoLoginCheck->setChecked(true);
    }
}

// saveLoginSettings 函數
void LoginWidget::saveLoginSettings()
{
    QSettings settings("config.ini", QSettings::IniFormat);

    bool remember = rememberMeCheck->isChecked();
    bool autoLogin = autoLoginCheck->isChecked();

    settings.setValue("Login/RememberMe", remember);
    settings.setValue("Login/AutoLogin", autoLogin);

    if (remember) {
        settings.setValue("Login/Username", usernameEdit->text());
        settings.setValue("Login/Password", passwordEdit->text().toUtf8().toBase64());
    } else {
        settings.remove("Login/Username");
        settings.remove("Login/Password");
    }
    settings.sync();
}

// 【新增】實現關閉事件
void LoginWidget::closeEvent(QCloseEvent *event)
{
    // 無論是否登錄成功，只要記住密碼被勾選，關閉窗口時都保存當前輸入的內容
    if (rememberMeCheck->isChecked()) {
        saveLoginSettings();
    }

    // 調用父類處理（確保窗口正常關閉）
    CFrameLessWidgetBase::closeEvent(event);
}
