#include "csystemsettingswidget.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QNetworkInterface>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

CSystemSettingsWidget::CSystemSettingsWidget(QWidget *parent) : QWidget(parent)
{
    // 使用 INI 文件格式存储配置，方便查看
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);
    setupUi();
    initLocalIps();
    loadSettings();
}

void CSystemSettingsWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // 通用样式
    QString groupStyle = "QGroupBox { color: white; font-weight: bold; border: 1px solid #505050; margin-top: 10px; }"
                         "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; }";
    QString labelStyle = "QLabel { color: #cccccc; font-weight: normal; }";
    QString editStyle = "QLineEdit, QComboBox { background-color: #3e3e3e; color: white; border: 1px solid #505050; padding: 5px; border-radius: 3px; }";

    // --- 1. 设备连接配置 ---
    QGroupBox *deviceGroup = new QGroupBox("摄像头连接配置", this);
    deviceGroup->setStyleSheet(groupStyle);
    QFormLayout *deviceLayout = new QFormLayout(deviceGroup);
    deviceLayout->setLabelAlignment(Qt::AlignRight);
    deviceLayout->setSpacing(15);

    m_deviceIpEdit = new QLineEdit(this);
    m_deviceIpEdit->setStyleSheet(editStyle);
    m_deviceUserEdit = new QLineEdit(this);
    m_deviceUserEdit->setStyleSheet(editStyle);
    m_devicePwdEdit = new QLineEdit(this);
    m_devicePwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    m_devicePwdEdit->setStyleSheet(editStyle);

    QLabel *l1 = new QLabel("设备IP地址:", this); l1->setStyleSheet(labelStyle);
    QLabel *l2 = new QLabel("用户名:", this); l2->setStyleSheet(labelStyle);
    QLabel *l3 = new QLabel("密码:", this); l3->setStyleSheet(labelStyle);

    deviceLayout->addRow(l1, m_deviceIpEdit);
    deviceLayout->addRow(l2, m_deviceUserEdit);
    deviceLayout->addRow(l3, m_devicePwdEdit);

    mainLayout->addWidget(deviceGroup);

    // --- 2. 报警回调配置 ---
    QGroupBox *callbackGroup = new QGroupBox("报警回调服务配置", this);
    callbackGroup->setStyleSheet(groupStyle);
    QFormLayout *cbLayout = new QFormLayout(callbackGroup);
    cbLayout->setLabelAlignment(Qt::AlignRight);
    cbLayout->setSpacing(15);

    m_localIpCombo = new QComboBox(this);
    m_localIpCombo->setStyleSheet(editStyle);
    m_localPortEdit = new QLineEdit(this);
    m_localPortEdit->setValidator(new QIntValidator(1024, 65535, this)); // 限制端口范围
    m_localPortEdit->setStyleSheet(editStyle);

    QLabel *l4 = new QLabel("本机接收IP:", this); l4->setStyleSheet(labelStyle);
    QLabel *l5 = new QLabel("监听端口:", this); l5->setStyleSheet(labelStyle);

    cbLayout->addRow(l4, m_localIpCombo);
    cbLayout->addRow(l5, m_localPortEdit);

    mainLayout->addWidget(callbackGroup);

    // --- 3. 底部按钮 ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnSave = new QPushButton("保存配置并应用", this);
    btnSave->setFixedSize(150, 40);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet("QPushButton { background-color: #0078d7; color: white; border-radius: 4px; font-weight: bold; font-size: 14px; }"
                           "QPushButton:hover { background-color: #008cfa; }"
                           "QPushButton:pressed { background-color: #0063b1; }");
    connect(btnSave, &QPushButton::clicked, this, &CSystemSettingsWidget::onSaveClicked);

    btnLayout->addWidget(btnSave);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch(); // 底部弹簧
}

void CSystemSettingsWidget::initLocalIps()
{
    m_localIpCombo->clear();
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            m_localIpCombo->addItem(entry.toString());
        }
    }
    if(m_localIpCombo->count() == 0) m_localIpCombo->addItem("127.0.0.1");
}

void CSystemSettingsWidget::loadSettings()
{
    // 读取配置，如果不存在则使用默认值
    m_deviceIpEdit->setText(m_settings->value("Device/IP", "192.168.6.100").toString());
    m_deviceUserEdit->setText(m_settings->value("Device/User", "admin").toString());
    m_devicePwdEdit->setText(m_settings->value("Device/Pwd", "admin").toString());

    QString savedLocalIp = m_settings->value("Callback/LocalIP").toString();
    int index = m_localIpCombo->findText(savedLocalIp);
    if(index != -1) m_localIpCombo->setCurrentIndex(index);

    m_localPortEdit->setText(m_settings->value("Callback/Port", "9999").toString());
}

void CSystemSettingsWidget::onSaveClicked()
{
    m_settings->setValue("Device/IP", m_deviceIpEdit->text());
    m_settings->setValue("Device/User", m_deviceUserEdit->text());
    m_settings->setValue("Device/Pwd", m_devicePwdEdit->text());

    m_settings->setValue("Callback/LocalIP", m_localIpCombo->currentText());
    m_settings->setValue("Callback/Port", m_localPortEdit->text());

    m_settings->sync(); // 强制写入文件

    QMessageBox::information(this, "系统提示", "配置已保存，正在重新应用...");
    emit sigConfigChanged(); // 通知外部重新加载配置
}

// Getters
QString CSystemSettingsWidget::getDeviceIp() const { return m_deviceIpEdit->text(); }
QString CSystemSettingsWidget::getDeviceUser() const { return m_deviceUserEdit->text(); }
QString CSystemSettingsWidget::getDevicePwd() const { return m_devicePwdEdit->text(); }
QString CSystemSettingsWidget::getLocalIp() const { return m_localIpCombo->currentText(); }
int CSystemSettingsWidget::getLocalPort() const { return m_localPortEdit->text().toInt(); }
