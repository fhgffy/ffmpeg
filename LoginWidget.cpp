#include "LoginWidget.h"
#include "StyleHelper.h"


#include <QMessageBox>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QTimer>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent), networkManager(new NetworkManager(this)), isLoading(false)
{
    setupUI();
    setupConnections();
    setFixedSize(400, 500);
}

LoginWidget::~LoginWidget()
{
}

void LoginWidget::setupUI()
{
    // 设置主布局
    loginLayout = new QVBoxLayout(this);
    loginLayout->setSpacing(0);  // 设置为0，让标题栏紧贴窗口边缘
    loginLayout->setContentsMargins(0, 0, 0, 0);  // 去掉所有边距

    // 创建内容部件，用于放置除标题栏外的所有内容
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(40, 20, 40, 40);  // 设置内容边距

    // 标题
    titleLabel = new QLabel("监控系统登录", contentWidget);
    titleLabel->setFont(StyleHelper::getTitleFont());
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #4299E1;"
                              "font-size: 36px;"
                              "font-weight: bold;");

    // 表单布局
    formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // IP地址输入框 - 可以设为只读或显示固定信息
    ipEdit = new QLineEdit(contentWidget);
    ipEdit->setPlaceholderText("服务端: 192.168.111.128(固定)");  // 修改提示信息
    ipEdit->setFont(StyleHelper::getInputFont());
    ipEdit->setMinimumHeight(45);
    ipEdit->setReadOnly(true);  // 设为只读，因为IP是固定的
    ipEdit->setText("192.168.111.128");  // 直接显示固定IP

        // 端口输入框 - 同样设为只读
    portEdit = new QLineEdit(contentWidget);
    portEdit->setPlaceholderText("端口: 8888 (固定)");  // 修改提示信息
    portEdit->setFont(StyleHelper::getInputFont());
    portEdit->setMinimumHeight(45);
    portEdit->setReadOnly(true);  // 设为只读
    portEdit->setText("8888");    // 直接显示固定端口

    // 用户名输入框
    usernameEdit = new QLineEdit(contentWidget);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setFont(StyleHelper::getInputFont());
    usernameEdit->setMinimumHeight(45);

    // 密码输入框
    passwordEdit = new QLineEdit(contentWidget);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setFont(StyleHelper::getInputFont());
    passwordEdit->setMinimumHeight(45);
    passwordEdit->setEchoMode(QLineEdit::Password);

    // 创建标签
    QLabel *ipLabel = new QLabel("IP地址:", contentWidget);
    QLabel *portLabel = new QLabel("端口:", contentWidget);
    QLabel *userLabel = new QLabel("用户名:", contentWidget);
    QLabel *passLabel = new QLabel("密码:", contentWidget);

    QVector<QLabel*> labels = {ipLabel, portLabel, userLabel, passLabel};
    for (auto label : labels) {
        label->setFont(StyleHelper::getLabelFont());
        label->setMinimumWidth(60);
    }

    // 添加到表单
    formLayout->addRow(ipLabel, ipEdit);
    formLayout->addRow(portLabel, portEdit);
    formLayout->addRow(userLabel, usernameEdit);
    formLayout->addRow(passLabel, passwordEdit);

    // 选项布局
    optionsLayout = new QHBoxLayout();
    rememberMeCheck = new QCheckBox("记住密码", contentWidget);
    autoLoginCheck = new QCheckBox("自动登录", contentWidget);

    rememberMeCheck->setFont(StyleHelper::getLabelFont());
    autoLoginCheck->setFont(StyleHelper::getLabelFont());

    optionsLayout->addWidget(rememberMeCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(autoLoginCheck);

    // 按钮布局
    buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    // 测试连接按钮
    testConnectionButton = new QPushButton("测试连接", contentWidget);
    testConnectionButton->setFont(StyleHelper::getButtonFont());

    // 注册按钮
    registerButton = new QPushButton("注册", contentWidget);
    registerButton->setFont(StyleHelper::getButtonFont());

    // 登录按钮
    loginButton = new QPushButton("登录", contentWidget);
    loginButton->setFont(StyleHelper::getButtonFont());
    loginButton->setDefault(true);

    buttonLayout->addWidget(testConnectionButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(loginButton);

    // 状态标签
    statusLabel = new QLabel(contentWidget);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFont(StyleHelper::getLabelFont());
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(40);

    // 将内容添加到内容布局
    contentLayout->addWidget(titleLabel);
    contentLayout->addLayout(formLayout);
    contentLayout->addLayout(optionsLayout);
    contentLayout->addWidget(createLine());
    contentLayout->addLayout(buttonLayout);
    contentLayout->addWidget(statusLabel);

    // 将内容部件添加到主布局
    loginLayout->addWidget(contentWidget);

    // 设置内容部件的样式
    contentWidget->setStyleSheet(StyleHelper::getAppStyleSheet());

    // 设置窗口样式
    setStyleSheet(StyleHelper::getAppStyleSheet());


}

QFrame* LoginWidget::createLine()
{
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: rgba(255, 255, 255, 0.2); height: 1px;");
    return line;
}

void LoginWidget::setupConnections()
{
    // 按钮点击信号
    connect(loginButton, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
    connect(testConnectionButton, &QPushButton::clicked, this, &LoginWidget::onTestConnectionClicked);

    // 复选框信号
    connect(rememberMeCheck, &QCheckBox::toggled, this, &LoginWidget::onRememberMeToggled);
    connect(autoLoginCheck, &QCheckBox::toggled, this, &LoginWidget::onAutoLoginToggled);

    // 网络管理器信号 - 使用lambda表达式接收信号
    // 注意：这里不再直接连接信号到onLoginResult等函数
    // 而是通过lambda表达式调用它们
    connect(networkManager, &NetworkManager::loginSuccess, this, [this]() {
        onLoginResult(true, "登录成功！");
    });

    connect(networkManager, &NetworkManager::loginFailed, this, [this](const QString &error) {
        onLoginResult(false, error);
    });

    connect(networkManager, &NetworkManager::registrationSuccess, this, [this]() {
        onRegisterResult(true, "注册成功！");
    });

    connect(networkManager, &NetworkManager::registrationFailed, this, [this](const QString &error) {
        onRegisterResult(false, error);
    });

    connect(networkManager, &NetworkManager::connectionTested, this, [this](bool success, const QString &message) {
        onConnectionTestResult(success, message);
    });

    // 输入框回车键触发登录
    connect(ipEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
    connect(portEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
}

// 修改验证逻辑，因为IP和端口固定了，只需要验证用户名和密码
void LoginWidget::onLoginClicked()
{
    qDebug() << "登录按钮被点击";
    if (isLoading) return;

    // 固定IP和端口 - 直接使用常量
    QString ip = "192.168.111.128";  // 固定服务端地址
    QString port = "8888";      // 固定端口
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    // 只验证用户名和密码（IP和端口固定）
    if (username.isEmpty() || password.isEmpty()) {
        showMessage("用户名和密码不能为空", true);
        return;
    }

    if (!Validators::isValidUsername(username)) {
        showMessage("用户名必须是4-20位字母数字", true);
        return;
    }

    if (!Validators::isValidPassword(password)) {
        showMessage("密码必须是6-20位，且包含字母和数字", true);
        return;
    }

    qDebug() << "连接服务端:" << ip << ":" << port;

    setLoading(true);
    showMessage("正在连接服务端...", false);

    // 发送登录请求信号（传递固定服务端IP和端口）
    emit loginRequested(ip, port.toInt(), username, password);

    // 使用实际的网络请求（连接到服务端）
    networkManager->login(ip, port.toInt(), username, password,
                         [this](bool success, const QString &message) {
        // 确保在主线程执行UI更新
        QMetaObject::invokeMethod(this, [this, success, message]() {
            setLoading(false);
            if (success) {
                showMessage("登录成功！", false);
                loginButton->setStyleSheet(
                    "background-color: #38A169; color: white; border: none; border-radius: 8px; padding: 14px;");
                // 这里可以添加登录成功后的跳转逻辑
            } else {
                showMessage("登录失败: " + message, true);
            }
        });
    });
}

void LoginWidget::onRegisterClicked()
{
    qDebug() << "注册按钮被点击";
    if (isLoading) return;

    // 固定IP和端口
    QString ip = "192.168.111.128";  // 固定服务端地址
    QString port = "8888";      // 固定端口

    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    // 只验证用户名和密码
    if (username.isEmpty() || password.isEmpty()) {
        showMessage("用户名和密码不能为空", true);
        return;
    }

    if (!Validators::isValidUsername(username)) {
        showMessage("用户名必须是4-20位字母数字", true);
        return;
    }

    if (!Validators::isValidPassword(password)) {
        showMessage("密码必须是6-20位，且包含字母和数字", true);
        return;
    }

    setLoading(true);
    showMessage("正在连接服务端...", false);

    // 发送注册请求信号
    emit registerRequested(ip, port.toInt(), username, password);

    // 使用实际的网络请求（连接到服务端）
    networkManager->registerUser(ip, port.toInt(), username, password,
                                [this](bool success, const QString &message) {
        // 确保在主线程执行UI更新
        QMetaObject::invokeMethod(this, [this, success, message]() {
            setLoading(false);
            if (success) {
                showMessage("注册成功！", false);
                registerButton->setStyleSheet(
                    "background-color: #38A169; color: white; border: none; border-radius: 8px; padding: 14px;");
                // 注册成功后可以自动切换到登录状态
            } else {
                showMessage("注册失败: " + message, true);
            }
        });
    });
}

void LoginWidget::onTestConnectionClicked()
{
    qDebug() << "测试连接按钮被点击";

    // 固定IP和端口
    QString ip = "192.168.111.128";  // 固定服务端地址
    QString port = "8888";      // 固定端口

    qDebug() << "测试连接服务端:" << ip << ":" << port;

    setLoading(true);
    showMessage("正在测试服务端连接...", false);

    // 发送测试连接信号
    emit connectionTestRequested(ip, port.toInt());

    // 使用实际的网络请求（测试服务端连接）
    networkManager->testConnection(ip, port.toInt(),
                                  [this](bool success, const QString &message) {
        setLoading(false);
        onConnectionTestResult(success, message);
    });
}

// 这些函数现在是私有成员函数，不是槽函数
void LoginWidget::onLoginResult(bool success, const QString &message)
{
    qDebug() << "登录结果:" << success << message;
    showMessage(message, !success);
    if (success) {
        loginButton->setStyleSheet(
            "background-color: #38A169; color: white; border: none; border-radius: 8px; padding: 14px;");
    }
    // 延时500毫秒，让用户看到“登录成功”的提示后再跳转
            QTimer::singleShot(500, this, [this](){
                emit sigLoginSuccess(); // 发送信号通知 main.cpp
                this->close();          // 关闭登录窗口
            });
}

void LoginWidget::onRegisterResult(bool success, const QString &message)
{
    qDebug() << "注册结果:" << success << message;
    showMessage(message, !success);
    if (success) {
        registerButton->setStyleSheet(
            "background-color: #38A169; color: white; border: none; border-radius: 8px; padding: 14px;");
    }
}

void LoginWidget::onConnectionTestResult(bool success, const QString &message)
{
    qDebug() << "连接测试结果:" << success << message;
    showMessage(message, !success);
    if (success) {
        testConnectionButton->setStyleSheet(
            "background-color: #38A169; color: white; border: none; border-radius: 8px; padding: 14px;");
    }
}

void LoginWidget::onRememberMeToggled(bool checked)
{
    qDebug() << "记住密码:" << checked;
    Q_UNUSED(checked);
}

void LoginWidget::onAutoLoginToggled(bool checked)
{
    qDebug() << "自动登录:" << checked;
    Q_UNUSED(checked);
}

void LoginWidget::showMessage(const QString &message, bool isError)
{
    statusLabel->setText(message);
    if (isError) {
        statusLabel->setStyleSheet("color: #FC8181;");
    } else {
        statusLabel->setStyleSheet("color: #68D391;");
    }
}

void LoginWidget::setLoading(bool loading)
{
    isLoading = loading;

    ipEdit->setEnabled(!loading);
    portEdit->setEnabled(!loading);
    usernameEdit->setEnabled(!loading);
    passwordEdit->setEnabled(!loading);
    loginButton->setEnabled(!loading);
    registerButton->setEnabled(!loading);
    testConnectionButton->setEnabled(!loading);
    rememberMeCheck->setEnabled(!loading);
    autoLoginCheck->setEnabled(!loading);

    if (loading) {
        loginButton->setText("登录中...");
        registerButton->setText("注册中...");
        testConnectionButton->setText("测试中...");
    } else {
        loginButton->setText("登录");
        registerButton->setText("注册");
        testConnectionButton->setText("测试连接");
    }
}
