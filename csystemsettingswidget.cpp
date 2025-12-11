#include "csystemsettingswidget.h"
#include "cryptstring.h" // 引入签名工具
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
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

CSystemSettingsWidget::CSystemSettingsWidget(QWidget *parent) : QWidget(parent)
{
    // 使用 INI 文件格式存储配置，方便查看
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);
    m_netManager = new QNetworkAccessManager(this); // 初始化网络管理器
    setupUi();
    initLocalIps();
    loadSettings();
}

void CSystemSettingsWidget::setupUi()
{
    // 使用 QScrollArea 防止内容过多显示不全，或者直接扩大主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // --- 样式修复核心代码 ---
    // 1. padding-top: 25px; -> 让里面的控件往下移，不要顶着标题
    // 2. background-color: #2d2d2d; -> 给标题加背景色，遮挡住穿过文字的横线
    QString groupStyle = "QGroupBox { color: white; font-weight: bold; border: 1px solid #505050; margin-top: 12px; padding-top: 25px; }"
                         "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: #2d2d2d; }";

    QString labelStyle = "QLabel { color: #cccccc; font-weight: normal; }";
    QString editStyle = "QLineEdit, QComboBox, QSpinBox { background-color: #3e3e3e; color: white; border: 1px solid #505050; padding: 4px; border-radius: 3px; }";

    // --- 1. 设备连接配置 ---
    QGroupBox *deviceGroup = new QGroupBox("摄像头连接配置", this);
    deviceGroup->setStyleSheet(groupStyle);
    QFormLayout *deviceLayout = new QFormLayout(deviceGroup);
    deviceLayout->setLabelAlignment(Qt::AlignRight);

    m_deviceIpEdit = new QLineEdit(this); m_deviceIpEdit->setStyleSheet(editStyle);
    m_deviceUserEdit = new QLineEdit(this); m_deviceUserEdit->setStyleSheet(editStyle);
    m_devicePwdEdit = new QLineEdit(this); m_devicePwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit); m_devicePwdEdit->setStyleSheet(editStyle);

    deviceLayout->addRow(new QLabel("设备IP:", this), m_deviceIpEdit);
    deviceLayout->addRow(new QLabel("用户名:", this), m_deviceUserEdit);
    deviceLayout->addRow(new QLabel("密码:", this), m_devicePwdEdit);
    mainLayout->addWidget(deviceGroup);

    // --- 2. 图像参数设置 (新增) ---
    QGroupBox *imgGroup = new QGroupBox("图像参数设置 (修改会重启推流)", this);
    imgGroup->setStyleSheet(groupStyle);
    QGridLayout *imgLayout = new QGridLayout(imgGroup);
    imgLayout->setSpacing(10);

    // 辅助函数：创建滑块行
    auto createSliderRow = [&](const QString &name, int min, int max, int val, int row, QSlider*& sliderPtr) {
        QLabel *lbl = new QLabel(name, this); lbl->setStyleSheet(labelStyle);
        sliderPtr = new QSlider(Qt::Horizontal, this);
        sliderPtr->setRange(min, max);
        sliderPtr->setValue(val);
        // 简单样式
        sliderPtr->setStyleSheet("QSlider::groove:horizontal { height: 4px; background: #505050; } QSlider::handle:horizontal { background: #E6A23C; width: 12px; margin: -4px 0; border-radius: 6px; }");

        QLabel *valLbl = new QLabel(QString::number(val), this);
        valLbl->setStyleSheet("color: white; width: 30px;");
        connect(sliderPtr, &QSlider::valueChanged, valLbl, [valLbl](int v){ valLbl->setText(QString::number(v)); });

        imgLayout->addWidget(lbl, row, 0);
        imgLayout->addWidget(sliderPtr, row, 1);
        imgLayout->addWidget(valLbl, row, 2);
    };

    // 第一列：基础画质
    createSliderRow("亮度", 0, 255, 128, 0, m_sliderBrightness);
    createSliderRow("对比度", 0, 255, 128, 1, m_sliderContrast);
    createSliderRow("饱和度", 0, 255, 128, 2, m_sliderSaturation);
    createSliderRow("锐度", 0, 255, 128, 3, m_sliderSharpness);

    // 第二列：高级参数
    // 曝光控制
    QLabel *lblExp = new QLabel("手动曝光:", this); lblExp->setStyleSheet(labelStyle);
    m_chkManualExposure = new QCheckBox(this);
    // 复选框样式稍微调整一下，使其在深色背景下更明显
    m_chkManualExposure->setStyleSheet("QCheckBox { color: white; } QCheckBox::indicator { width: 18px; height: 18px; }");

    imgLayout->addWidget(lblExp, 0, 3);
    imgLayout->addWidget(m_chkManualExposure, 0, 4);

    QLabel *lblExpTime = new QLabel("曝光时间:", this); lblExpTime->setStyleSheet(labelStyle);
    m_spinExposureTime = new QSpinBox(this);
    m_spinExposureTime->setRange(0, 5391);
    m_spinExposureTime->setStyleSheet(editStyle);
    imgLayout->addWidget(lblExpTime, 1, 3);
    imgLayout->addWidget(m_spinExposureTime, 1, 4);

    // 灯光控制
    QLabel *lblLightMode = new QLabel("补光模式:", this); lblLightMode->setStyleSheet(labelStyle);
    m_comboLightControl = new QComboBox(this);
    m_comboLightControl->addItems({"关闭", "红外补光", "白光灯", "自动"});
    m_comboLightControl->setStyleSheet(editStyle);
    imgLayout->addWidget(lblLightMode, 2, 3);
    imgLayout->addWidget(m_comboLightControl, 2, 4);

    // 强光抑制
    QLabel *lblDepress = new QLabel("强光抑制:", this); lblDepress->setStyleSheet(labelStyle);
    m_sliderLightDepress = new QSlider(Qt::Horizontal, this);
    m_sliderLightDepress->setRange(0, 255);
    m_sliderLightDepress->setStyleSheet("QSlider::groove:horizontal { height: 4px; background: #505050; } QSlider::handle:horizontal { background: #3daee9; width: 12px; margin: -4px 0; border-radius: 6px; }");
    imgLayout->addWidget(lblDepress, 3, 3);
    imgLayout->addWidget(m_sliderLightDepress, 3, 4);

    // 应用按钮
    QPushButton *btnApplyImg = new QPushButton("应用图像设置", this);
    btnApplyImg->setStyleSheet("QPushButton { background-color: #E6A23C; color: white; border-radius: 4px; padding: 6px; font-weight:bold; } QPushButton:hover { background-color: #eba94e; }");
    connect(btnApplyImg, &QPushButton::clicked, this, &CSystemSettingsWidget::onSaveImageParamsClicked);
    imgLayout->addWidget(btnApplyImg, 4, 3, 1, 2); // 跨两列

    mainLayout->addWidget(imgGroup);

    // --- 3. 报警回调配置 ---
    QGroupBox *callbackGroup = new QGroupBox("报警回调服务配置", this);
    callbackGroup->setStyleSheet(groupStyle);
    QFormLayout *cbLayout = new QFormLayout(callbackGroup);

    m_localIpCombo = new QComboBox(this); m_localIpCombo->setStyleSheet(editStyle);
    m_localPortEdit = new QLineEdit(this); m_localPortEdit->setStyleSheet(editStyle);
    m_localPortEdit->setValidator(new QIntValidator(1024, 65535, this));

    cbLayout->addRow(new QLabel("本机IP:", this), m_localIpCombo);
    cbLayout->addRow(new QLabel("监听端口:", this), m_localPortEdit);
    mainLayout->addWidget(callbackGroup);

    // --- 底部保存按钮 (原有) ---
    QPushButton *btnSave = new QPushButton("保存系统配置", this);
    btnSave->setFixedHeight(40);
    btnSave->setStyleSheet("QPushButton { background-color: #0078d7; color: white; border-radius: 4px; font-weight: bold; font-size:14px; } QPushButton:hover { background-color: #008cfa; }");
    connect(btnSave, &QPushButton::clicked, this, &CSystemSettingsWidget::onSaveClicked);
    mainLayout->addWidget(btnSave);

    mainLayout->addStretch();
}

void CSystemSettingsWidget::onSaveImageParamsClicked()
{
    // 构造参数 Map，文档要求 key1, value1, type1...
    QMap<QString, QString> settings;

    // 1. 基础画质
    settings.insert("encode.0.vi_param.0.brightness", QString::number(m_sliderBrightness->value()));
    settings.insert("encode.0.vi_param.0.contrast", QString::number(m_sliderContrast->value()));
    settings.insert("encode.0.vi_param.0.saturation", QString::number(m_sliderSaturation->value()));
    settings.insert("encode.0.vi_param.0.sharpness", QString::number(m_sliderSharpness->value()));

    // 2. 曝光与灯光
    settings.insert("encode.0.vi_param.0.manualExposure", m_chkManualExposure->isChecked() ? "1" : "0");
    settings.insert("encode.0.vi_param.0.exposure", QString::number(m_spinExposureTime->value()));
    settings.insert("encode.0.vi_param.0.lightControlMode", QString::number(m_comboLightControl->currentIndex())); // 0-3 对应 combo 索引
    settings.insert("encode.0.vi_param.0.lightDepress", QString::number(m_sliderLightDepress->value()));

    // 发送请求
    sendApiRequest(settings);
}

void CSystemSettingsWidget::sendApiRequest(const QMap<QString, QString>& params)
{
    // 1. 准备 URL
    QString ip = m_deviceIpEdit->text();
    if(ip.isEmpty()) ip = "192.168.6.100"; // 默认
    QUrl url(QString("http://%1/xsw/modValues").arg(ip));

    // 2. 使用 KVQuery 构造带签名的参数
    KVQuery kv;

    // API 要求格式: key1=xxx&value1=xxx&type1=number&key2=...
    int index = 1;
    for(auto it = params.begin(); it != params.end(); ++it) {
        kv.add(QString("key%1").arg(index).toStdString(), it.key().toStdString());
        kv.add(QString("value%1").arg(index).toStdString(), it.value().toStdString());
        kv.add(QString("type%1").arg(index).toStdString(), "number");

        index++;
        if(index > 10) break; // API 限制一次最多10个
    }

    // 3. 生成带 token 的查询字符串
    QString queryString = QString::fromStdString(kv.toCrpytString());

    // 4. 设置到 URL Query 中
    url.setQuery(queryString);

    // 5. 发送 POST
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_netManager->post(request, QByteArray()); // Body 为空

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if(reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "成功", "参数下发成功，设备可能正在重启推流...");
        } else {
            QMessageBox::warning(this, "失败", "设置失败: " + reply->errorString());
        }
        reply->deleteLater();
    });
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
    // 读取配置
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

    m_settings->sync();

    QMessageBox::information(this, "系统提示", "基本连接配置已保存");
    emit sigConfigChanged();
}

QString CSystemSettingsWidget::getDeviceIp() const { return m_deviceIpEdit->text(); }
QString CSystemSettingsWidget::getDeviceUser() const { return m_deviceUserEdit->text(); }
QString CSystemSettingsWidget::getDevicePwd() const { return m_devicePwdEdit->text(); }
QString CSystemSettingsWidget::getLocalIp() const { return m_localIpCombo->currentText(); }
int CSystemSettingsWidget::getLocalPort() const { return m_localPortEdit->text().toInt(); }
