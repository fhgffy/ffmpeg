#include "cptzcontrolwidget.h"
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

void CPTZControlWidget::onBtnCenterClicked() {
    qDebug() << "Center/Stop Clicked -> Emitting Signal"; // 添加日志方便调试
    sendPtzRequest("s");  // Send PTZ request for center/stop
    emit sig_CenterClicked();  // 发出信号
}

void CPTZControlWidget::sendPtzRequest(const QString &command)
{
    // Construct the API URL
    QUrl url("http://192.168.6.100/xsw/api/ptz/control");
    
    // Build query parameters
    QUrlQuery query;
    query.addQueryItem("value", command);
    query.addQueryItem("stop", "1");
    query.addQueryItem("steps", QString::number(m_step));
    
    // Set the query on the URL
    url.setQuery(query);
    
    // Create the request
    QNetworkRequest request(url);
    
    // Send POST request
    QNetworkReply *reply = m_networkManager->post(request, QByteArray());
    
    // Log the request for debugging
    qDebug() << "PTZ Request:" << url.toString();
    
    // Connect to handle the reply (optional - for error handling)
    connect(reply, &QNetworkReply::finished, this, [this]() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
        if (reply) {
            if (reply->error() == QNetworkReply::NoError) {
                qDebug() << "PTZ Request successful:" << reply->readAll();
            } else {
                qDebug() << "PTZ Request error:" << reply->errorString();
            }
            reply->deleteLater();
        }
    });
}

CPTZControlWidget::CPTZControlWidget(QWidget *parent) : QWidget(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_step = 5;  // Default step value
    setupUi();
}

void CPTZControlWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. 标题
    QLabel *titleLabel = new QLabel(tr("云台控制"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(30);
    titleLabel->setStyleSheet("background-color: #2d2d2d; color: white; font-weight: bold; border-bottom: 1px solid #505050;");
    mainLayout->addWidget(titleLabel);

    // 2. 控制盘容器
    QWidget *controlArea = new QWidget(this);
    // 背景图控制盘
    controlArea->setStyleSheet(
        "QWidget#ControlArea { "
        "   border-image: url(:/control/control.png) 0 0 0 0 stretch stretch; "
        "}"
    );
    controlArea->setObjectName("ControlArea");

    QGridLayout *gridLayout = new QGridLayout(controlArea);
    gridLayout->setContentsMargins(30, 30, 30, 30); // 调整边距以匹配图片按钮位置
    gridLayout->setSpacing(5);

    // 辅助lambda：创建透明方向按钮
    auto createDirBtn = [this](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // 设置为透明，覆盖在背景图上
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: transparent; "
            "   border: none; "
            "}"
            "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.1); }" // 按下稍微变亮一点作为反馈
        );
        return btn;
    };

    // --- 核心修复：添加9个方向按钮 ---

    // 第一行
    QPushButton *btnUpLeft = createDirBtn("");
    connect(btnUpLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpLeftClicked);
    gridLayout->addWidget(btnUpLeft, 0, 0);

    QPushButton *btnUp = createDirBtn("");
    connect(btnUp, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpClicked);
    gridLayout->addWidget(btnUp, 0, 1);

    QPushButton *btnUpRight = createDirBtn("");
    connect(btnUpRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpRightClicked);
    gridLayout->addWidget(btnUpRight, 0, 2);

    // 第二行
    QPushButton *btnLeft = createDirBtn("");
    connect(btnLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnLeftClicked);
    gridLayout->addWidget(btnLeft, 1, 0);

    // [重点] 中间按钮 - 连接到 onBtnCenterClicked
    QPushButton *btnCenter = createDirBtn("");
    btnCenter->setToolTip("反转视频"); // 鼠标悬停提示
    connect(btnCenter, &QPushButton::clicked, this, &CPTZControlWidget::onBtnCenterClicked);
    gridLayout->addWidget(btnCenter, 1, 1);

    QPushButton *btnRight = createDirBtn("");
    connect(btnRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnRightClicked);
    gridLayout->addWidget(btnRight, 1, 2);

    // 第三行
    QPushButton *btnDownLeft = createDirBtn("");
    connect(btnDownLeft, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownLeftClicked);
    gridLayout->addWidget(btnDownLeft, 2, 0);

    QPushButton *btnDown = createDirBtn("");
    connect(btnDown, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownClicked);
    gridLayout->addWidget(btnDown, 2, 1);

    QPushButton *btnDownRight = createDirBtn("");
    connect(btnDownRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownRightClicked);
    gridLayout->addWidget(btnDownRight, 2, 2);

    // -------------------------------

    mainLayout->addWidget(controlArea, 2);

    // 3. 功能控制区域 (保持你原有的代码)
    QWidget *functionArea = new QWidget(this);
    functionArea->setStyleSheet("background-color: #2d2d2d;");
    QGridLayout *funcLayout = new QGridLayout(functionArea);
    funcLayout->setContentsMargins(15, 10, 15, 10);
    funcLayout->setSpacing(10);
    funcLayout->setVerticalSpacing(15);

    auto createLabel = [this](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet("color: #cccccc; font-size: 12px;");
        return lbl;
    };

    auto createFuncBtn = [this](bool isAdd) -> QPushButton* {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(28, 28);
        btn->setCursor(Qt::PointingHandCursor);
        QString imgPath = isAdd ? ":/condition/plus.png" : ":/condition/min.png";
        btn->setStyleSheet(QString(
            "QPushButton { border-image: url(%1); border: none; }"
            "QPushButton:hover { opacity: 0.8; }"
            "QPushButton:pressed { padding-top: 2px; }"
        ).arg(imgPath));
        return btn;
    };

    // 第一行：变倍
    funcLayout->addWidget(createLabel("变倍"), 0, 0);
    QPushButton *btnZoomIn = createFuncBtn(true);
    QPushButton *btnZoomOut = createFuncBtn(false);
    connect(btnZoomIn, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomInClicked);
    connect(btnZoomOut, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomOutClicked);
    funcLayout->addWidget(btnZoomIn, 0, 1);
    funcLayout->addWidget(btnZoomOut, 0, 2);

    // 第二行：聚焦
    funcLayout->addWidget(createLabel("聚焦"), 1, 0);
    QPushButton *btnFocusNear = createFuncBtn(true);
    QPushButton *btnFocusFar = createFuncBtn(false);
    connect(btnFocusNear, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusNearClicked);
    connect(btnFocusFar, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusFarClicked);
    funcLayout->addWidget(btnFocusNear, 1, 1);
    funcLayout->addWidget(btnFocusFar, 1, 2);

    // 第三行：光圈
    funcLayout->addWidget(createLabel("光圈"), 2, 0);
    QPushButton *btnApertureOpen = createFuncBtn(true);
    QPushButton *btnApertureClose = createFuncBtn(false);
    connect(btnApertureOpen, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureOpenClicked);
    connect(btnApertureClose, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureCloseClicked);
    funcLayout->addWidget(btnApertureOpen, 2, 1);
    funcLayout->addWidget(btnApertureClose, 2, 2);

    // 第四行：步长
    funcLayout->addWidget(createLabel("步长"), 3, 0);
    QSlider *stepSlider = new QSlider(Qt::Horizontal, this);
    stepSlider->setRange(1, 10);
    stepSlider->setValue(5);
    stepSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #505050; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #3daee9; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
        "QSlider::sub-page:horizontal { background: #3daee9; border-radius: 2px; }"
    );
    QLabel *stepValueLabel = new QLabel("5", this);
    stepValueLabel->setStyleSheet("color: white; font-weight: bold;");
    stepValueLabel->setFixedWidth(20);
    stepValueLabel->setAlignment(Qt::AlignCenter);
    connect(stepSlider, &QSlider::valueChanged, this, [=](int val){
        stepValueLabel->setText(QString::number(val));
        onStepChanged(val);
    });
    funcLayout->addWidget(stepSlider, 3, 1);
    funcLayout->addWidget(stepValueLabel, 3, 2);

    mainLayout->addWidget(functionArea, 3);
}

// 槽函数实现
void CPTZControlWidget::onBtnUpClicked() { 
    qDebug() << "Up"; 
    sendPtzRequest("u");
}
void CPTZControlWidget::onBtnDownClicked() { 
    qDebug() << "Down"; 
    sendPtzRequest("d");
}
void CPTZControlWidget::onBtnLeftClicked() { 
    qDebug() << "Left"; 
    sendPtzRequest("l");
}
void CPTZControlWidget::onBtnRightClicked() { 
    qDebug() << "Right"; 
    sendPtzRequest("r");
}
void CPTZControlWidget::onBtnUpLeftClicked() { 
    qDebug() << "UpLeft"; 
    sendPtzRequest("1");
}
void CPTZControlWidget::onBtnUpRightClicked() { 
    qDebug() << "UpRight"; 
    sendPtzRequest("2");
}
void CPTZControlWidget::onBtnDownLeftClicked() { 
    qDebug() << "DownLeft"; 
    sendPtzRequest("3");
}
void CPTZControlWidget::onBtnDownRightClicked() { 
    qDebug() << "DownRight"; 
    sendPtzRequest("4");
}
void CPTZControlWidget::onBtnZoomInClicked() { 
    qDebug() << "Zoom In"; 
    sendPtzRequest("f");
}
void CPTZControlWidget::onBtnZoomOutClicked() { 
    qDebug() << "Zoom Out"; 
    sendPtzRequest("n");
}
void CPTZControlWidget::onBtnFocusNearClicked() { 
    qDebug() << "Focus Near"; 
    sendPtzRequest("i");
}
void CPTZControlWidget::onBtnFocusFarClicked() { 
    qDebug() << "Focus Far"; 
    sendPtzRequest("o");
}
void CPTZControlWidget::onBtnApertureOpenClicked() { qDebug() << "Aperture Open"; }
void CPTZControlWidget::onBtnApertureCloseClicked() { qDebug() << "Aperture Close"; }
void CPTZControlWidget::onStepChanged(int value) { 
    qDebug() << "Step:" << value; 
    m_step = value;
}
