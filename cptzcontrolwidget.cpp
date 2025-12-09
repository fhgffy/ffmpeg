#include "cptzcontrolwidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

CPTZControlWidget::CPTZControlWidget(QWidget *parent) : QWidget(parent)
{
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

    // 关键修正：将背景图设置在容器 controlArea 上，使用 border-image 拉伸铺满，或者 background-image 居中
    // 这里使用 border-image 保证图片跟随控件大小缩放
    controlArea->setStyleSheet(
        "QWidget#ControlArea { "
        "   border-image: url(:/control/control.png) 0 0 0 0 stretch stretch; "
        "}"
    );
    controlArea->setObjectName("ControlArea"); // 设置对象名以匹配上面的样式选择器

    QGridLayout *gridLayout = new QGridLayout(controlArea);
    // 调整边距，让按钮与背景图的箭头位置对齐。您可以根据实际效果微调这里的数值。
    gridLayout->setContentsMargins(30, 30, 30, 30);
    gridLayout->setSpacing(5);

    // 创建透明方向按钮的辅助 lambda
    auto createDirBtn = [this](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // 样式：背景透明，白色文字，鼠标悬停时略微发亮
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: transparent; "
            "   color: rgba(255, 255, 255, 0.9); "
            "   font-size: 24px; "   // 字体大一点
            "   border: none; "
            "   font-weight: bold;"
            "}"
            "QPushButton:hover { color: #0078d7; }" // 悬停变蓝
            "QPushButton:pressed { color: #005a9e; padding-top: 2px; }" // 按下效果
        );
        return btn;
    };

    // 8个方向按钮 (使用 Unicode 字符)
    QPushButton *btnUp        = createDirBtn("");
    QPushButton *btnDown      = createDirBtn("");
    QPushButton *btnLeft      = createDirBtn("");
    QPushButton *btnRight     = createDirBtn("");
    QPushButton *btnUpLeft    = createDirBtn("");
    QPushButton *btnUpRight   = createDirBtn("");
    QPushButton *btnDownLeft  = createDirBtn("");
    QPushButton *btnDownRight = createDirBtn("");

    // 中间翻转按钮 (使用图片)
    QPushButton *btnCenter = new QPushButton("", this);
       btnCenter->setCursor(Qt::PointingHandCursor);
       btnCenter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

       // 样式：背景透明，文字居中，颜色与周围按钮一致
       btnCenter->setStyleSheet(
           "QPushButton { "
           "   background-color: transparent; "
           "   color: rgba(255, 255, 255, 0.9); "
           "   font-size: 24px; "   // 字体大小
           "   border: none; "
           "   font-weight: bold;"
           "}"
           "QPushButton:hover { color: #0078d7; }" // 悬停变蓝
           "QPushButton:pressed { color: #005a9e; }" // 按下变深蓝
       );

    // 布局添加
    gridLayout->addWidget(btnUpLeft,    0, 0);
    gridLayout->addWidget(btnUp,        0, 1);
    gridLayout->addWidget(btnUpRight,   0, 2);

    gridLayout->addWidget(btnLeft,      1, 0);
    gridLayout->addWidget(btnCenter,    1, 1);
    gridLayout->addWidget(btnRight,     1, 2);

    gridLayout->addWidget(btnDownLeft,  2, 0);
    gridLayout->addWidget(btnDown,      2, 1);
    gridLayout->addWidget(btnDownRight, 2, 2);

    mainLayout->addWidget(controlArea);
    mainLayout->addStretch(); // 底部填充

    // 连接信号
    connect(btnUp,        &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpClicked);
    connect(btnDown,      &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownClicked);
    connect(btnLeft,      &QPushButton::clicked, this, &CPTZControlWidget::onBtnLeftClicked);
    connect(btnRight,     &QPushButton::clicked, this, &CPTZControlWidget::onBtnRightClicked);
    connect(btnUpLeft,    &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpLeftClicked);
    connect(btnUpRight,   &QPushButton::clicked, this, &CPTZControlWidget::onBtnUpRightClicked);
    connect(btnDownLeft,  &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownLeftClicked);
    connect(btnDownRight, &QPushButton::clicked, this, &CPTZControlWidget::onBtnDownRightClicked);
    connect(btnCenter,    &QPushButton::clicked, this, &CPTZControlWidget::onBtnCenterClicked);
}

void CPTZControlWidget::onBtnUpClicked()       { qDebug() << "PTZ: UP"; }
void CPTZControlWidget::onBtnDownClicked()     { qDebug() << "PTZ: DOWN"; }
void CPTZControlWidget::onBtnLeftClicked()     { qDebug() << "PTZ: LEFT"; }
void CPTZControlWidget::onBtnRightClicked()    { qDebug() << "PTZ: RIGHT"; }
void CPTZControlWidget::onBtnUpLeftClicked()   { qDebug() << "PTZ: UP_LEFT"; }
void CPTZControlWidget::onBtnUpRightClicked()  { qDebug() << "PTZ: UP_RIGHT"; }
void CPTZControlWidget::onBtnDownLeftClicked() { qDebug() << "PTZ: DOWN_LEFT"; }
void CPTZControlWidget::onBtnDownRightClicked(){ qDebug() << "PTZ: DOWN_RIGHT"; }
void CPTZControlWidget::onBtnCenterClicked()   { qDebug() << "PTZ: INVERSE/FLIP"; }
