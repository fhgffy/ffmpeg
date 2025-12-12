#include "cptzcontrolwidget.h"
#include "cryptstring.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>

CPTZControlWidget::CPTZControlWidget(QWidget *parent) : QWidget(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_step = 5;  // Default step value
    m_stopTimer = new QTimer(this);
    m_stopTimer->setSingleShot(true); // 只触发一次
    connect(m_stopTimer, &QTimer::timeout, this, [this](){
        qDebug() << "自动发送 stop s！";
        sendPtzRequest("s");
    });

    setupUi();
}

void CPTZControlWidget::sendPtzRequest(const QString &command)
{
    //
    QUrl url("http://192.168.6.100/xsw/api/ptz/control");

    KVQuery kvQuery;
    kvQuery.add("value", command.toStdString());
    kvQuery.add("stop", "1");
    kvQuery.add("steps", std::to_string(m_step));

    std::string queryString = kvQuery.toCrpytString();
    url.setQuery(QString::fromStdString(queryString));

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->post(request, QByteArray());

    // 简单处理内存泄露
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

// ================= 方向控制槽函数 (添加日志信号) =================

void CPTZControlWidget::onBtnUpClicked() {
    emit sigMessage("云台控制: 向上转动"); // 【新增】
    sendPtzRequest("u");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnDownClicked() {
    emit sigMessage("云台控制: 向下转动"); // 【新增】
    sendPtzRequest("d");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnLeftClicked() {
    emit sigMessage("云台控制: 向左转动"); // 【新增】
    sendPtzRequest("l");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnRightClicked() {
    emit sigMessage("云台控制: 向右转动"); // 【新增】
    sendPtzRequest("r");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnUpLeftClicked() {
    emit sigMessage("云台控制: 向左上转动"); // 【新增】
    sendPtzRequest("1");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnUpRightClicked() {
    emit sigMessage("云台控制: 向右上转动"); // 【新增】
    sendPtzRequest("2");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnDownLeftClicked() {
    emit sigMessage("云台控制: 向左下转动"); // 【新增】
    sendPtzRequest("3");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnDownRightClicked() {
    emit sigMessage("云台控制: 向右下转动"); // 【新增】
    sendPtzRequest("4");
    m_stopTimer->start(1000);
}
void CPTZControlWidget::onBtnCenterClicked() {
    emit sigMessage("云台控制: 复位/反转"); // 【新增】
    sendPtzRequest("s");
    emit sig_CenterClicked();
}

// ================= 功能控制槽函数 (添加日志信号) =================

void CPTZControlWidget::onBtnZoomInClicked() {
    emit sigMessage("镜头控制: 变倍+ (放大)"); // 【新增】
    sendPtzRequest("f");
}
void CPTZControlWidget::onBtnZoomOutClicked() {
    emit sigMessage("镜头控制: 变倍- (缩小)"); // 【新增】
    sendPtzRequest("n");
}
void CPTZControlWidget::onBtnFocusNearClicked() {
    emit sigMessage("镜头控制: 聚焦+ (近)"); // 【新增】
    sendPtzRequest("i");
}
void CPTZControlWidget::onBtnFocusFarClicked() {
    emit sigMessage("镜头控制: 聚焦- (远)"); // 【新增】
    sendPtzRequest("o");
}
void CPTZControlWidget::onBtnApertureOpenClicked() {
    emit sigMessage("镜头控制: 光圈+ (开)"); // 【新增】
    // 预留接口
}
void CPTZControlWidget::onBtnApertureCloseClicked() {
    emit sigMessage("镜头控制: 光圈- (关)"); // 【新增】
    // 预留接口
}
void CPTZControlWidget::onStepChanged(int value) {
    // 步长改变不需要发网络请求，但记录日志
    emit sigMessage(QString("云台设置: 步长调整为 %1").arg(value)); // 【新增】
    m_step = value;
}

void CPTZControlWidget::setupUi()
{
    // ... UI 代码保持原样，未修改 ...
    // (为了节省篇幅，这里复用您上传文件中的 setupUi 代码，逻辑不变)

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel(tr("云台控制"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(30);
    titleLabel->setStyleSheet("background-color: #2d2d2d; color: white; font-weight: bold; border-bottom: 1px solid #505050;");
    mainLayout->addWidget(titleLabel);

    QWidget *controlArea = new QWidget(this);
    controlArea->setStyleSheet("QWidget#ControlArea { border-image: url(:/control/control.png) 0 0 0 0 stretch stretch; }");
    controlArea->setObjectName("ControlArea");

    QGridLayout *gridLayout = new QGridLayout(controlArea);
    gridLayout->setContentsMargins(30, 30, 30, 30);
    gridLayout->setSpacing(5);

    auto createDirBtn = [this](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet("QPushButton { background-color: transparent; border: none; } QPushButton:pressed { background-color: rgba(255, 255, 255, 0.1); }");
        return btn;
    };

    // 绑定9个方向按钮
    QPushButton *btnUpLeft = createDirBtn(""); connect(btnUpLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpLeftClicked); gridLayout->addWidget(btnUpLeft, 0, 0);
    QPushButton *btnUp = createDirBtn(""); connect(btnUp, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpClicked); gridLayout->addWidget(btnUp, 0, 1);
    QPushButton *btnUpRight = createDirBtn(""); connect(btnUpRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpRightClicked); gridLayout->addWidget(btnUpRight, 0, 2);
    QPushButton *btnLeft = createDirBtn(""); connect(btnLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnLeftClicked); gridLayout->addWidget(btnLeft, 1, 0);
    QPushButton *btnCenter = createDirBtn(""); btnCenter->setToolTip("反转视频"); connect(btnCenter, &QPushButton::clicked, this, &CPTZControlWidget::onBtnCenterClicked); gridLayout->addWidget(btnCenter, 1, 1);
    QPushButton *btnRight = createDirBtn(""); connect(btnRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnRightClicked); gridLayout->addWidget(btnRight, 1, 2);
    QPushButton *btnDownLeft = createDirBtn(""); connect(btnDownLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownLeftClicked); gridLayout->addWidget(btnDownLeft, 2, 0);
    QPushButton *btnDown = createDirBtn(""); connect(btnDown, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownClicked); gridLayout->addWidget(btnDown, 2, 1);
    QPushButton *btnDownRight = createDirBtn(""); connect(btnDownRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownRightClicked); gridLayout->addWidget(btnDownRight, 2, 2);

    mainLayout->addWidget(controlArea, 2);

    // 功能区
    QWidget *functionArea = new QWidget(this);
    functionArea->setStyleSheet("background-color: #2d2d2d;");
    QGridLayout *funcLayout = new QGridLayout(functionArea);
    funcLayout->setContentsMargins(15, 10, 15, 10);
    funcLayout->setSpacing(10);
    funcLayout->setVerticalSpacing(15);

    auto createLabel = [this](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text, this); lbl->setStyleSheet("color: #cccccc; font-size: 12px;"); return lbl;
    };
    auto createFuncBtn = [this](bool isAdd) -> QPushButton* {
        QPushButton *btn = new QPushButton(this); btn->setFixedSize(28, 28); btn->setCursor(Qt::PointingHandCursor);
        QString imgPath = isAdd ? ":/condition/plus.png" : ":/condition/min.png";
        btn->setStyleSheet(QString("QPushButton { border-image: url(%1); border: none; } QPushButton:hover { opacity: 0.8; } QPushButton:pressed { padding-top: 2px; }").arg(imgPath));
        return btn;
    };

    funcLayout->addWidget(createLabel("变倍"), 0, 0);
    QPushButton *btnZoomIn = createFuncBtn(true); connect(btnZoomIn, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomInClicked);
    QPushButton *btnZoomOut = createFuncBtn(false); connect(btnZoomOut, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomOutClicked);
    funcLayout->addWidget(btnZoomIn, 0, 1); funcLayout->addWidget(btnZoomOut, 0, 2);

    funcLayout->addWidget(createLabel("聚焦"), 1, 0);
    QPushButton *btnFocusNear = createFuncBtn(true); connect(btnFocusNear, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusNearClicked);
    QPushButton *btnFocusFar = createFuncBtn(false); connect(btnFocusFar, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusFarClicked);
    funcLayout->addWidget(btnFocusNear, 1, 1); funcLayout->addWidget(btnFocusFar, 1, 2);

    funcLayout->addWidget(createLabel("光圈"), 2, 0);
    QPushButton *btnApertureOpen = createFuncBtn(true); connect(btnApertureOpen, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureOpenClicked);
    QPushButton *btnApertureClose = createFuncBtn(false); connect(btnApertureClose, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureCloseClicked);
    funcLayout->addWidget(btnApertureOpen, 2, 1); funcLayout->addWidget(btnApertureClose, 2, 2);

    funcLayout->addWidget(createLabel("步长"), 3, 0);
    QSlider *stepSlider = new QSlider(Qt::Horizontal, this);
    stepSlider->setRange(1, 10); stepSlider->setValue(5);
    stepSlider->setStyleSheet("QSlider::groove:horizontal { height: 4px; background: #505050; border-radius: 2px; } QSlider::handle:horizontal { background: #3daee9; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; } QSlider::sub-page:horizontal { background: #3daee9; border-radius: 2px; }");
    QLabel *stepValueLabel = new QLabel("5", this);
    stepValueLabel->setStyleSheet("color: white; font-weight: bold;");
    stepValueLabel->setFixedWidth(20); stepValueLabel->setAlignment(Qt::AlignCenter);
    connect(stepSlider, &QSlider::valueChanged, this, [=](int val){ stepValueLabel->setText(QString::number(val)); onStepChanged(val); });
    funcLayout->addWidget(stepSlider, 3, 1); funcLayout->addWidget(stepValueLabel, 3, 2);

    mainLayout->addWidget(functionArea, 3);
}
