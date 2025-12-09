#include "cptzcontrolwidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
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

    // 2. 控制盘容器 (保持原有逻辑)
    QWidget *controlArea = new QWidget(this);
    controlArea->setStyleSheet(
        "QWidget#ControlArea { "
        "   border-image: url(:/control/control.png) 0 0 0 0 stretch stretch; "
        "}"
    );
    controlArea->setObjectName("ControlArea");

    QGridLayout *gridLayout = new QGridLayout(controlArea);
    gridLayout->setContentsMargins(30, 30, 30, 30);
    gridLayout->setSpacing(5);

    auto createDirBtn = [this](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: transparent; "
            "   color: rgba(255, 255, 255, 0.9); "
            "   font-size: 24px; "
            "   border: none; "
            "   font-weight: bold;"
            "}"
            "QPushButton:hover { color: #0078d7; }"
            "QPushButton:pressed { color: #005a9e; padding-top: 2px; }"
        );
        return btn;
    };

    // 此处省略方向按钮的具体添加代码(保持原样即可)，为了篇幅聚焦在新增部分
    // 如果需要可以把之前的方向键代码原样放回这里
    // ... (假设方向键代码已存在) ...

    mainLayout->addWidget(controlArea, 2); // 伸缩因子2，让罗盘占较大空间

    // 3. 功能控制区域 (新增部分)
    QWidget *functionArea = new QWidget(this);
    functionArea->setStyleSheet("background-color: #2d2d2d;");
    QGridLayout *funcLayout = new QGridLayout(functionArea);
    funcLayout->setContentsMargins(15, 10, 15, 10);
    funcLayout->setSpacing(10);
    funcLayout->setVerticalSpacing(15);

    // 辅助函数：创建功能行的 Label
    auto createLabel = [this](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet("color: #cccccc; font-size: 12px;");
        return lbl;
    };

    // 辅助函数：创建加减按钮 (加载 condition 目录下的图片)
    auto createFuncBtn = [this](bool isAdd) -> QPushButton* {
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(28, 28);
        btn->setCursor(Qt::PointingHandCursor);

        // 假设图片名称为 add.png 和 sub.png (根据您的描述修改具体文件名)
        QString imgPath = isAdd ? ":/condition/plus.png" : ":/condition/min.png";

        // 样式：圆角背景 + 图标
        btn->setStyleSheet(QString(
            "QPushButton { "
            "   border-image: url(%1); " // 使用图片作为背景
            "   border: none; "
            "}"
            "QPushButton:hover { opacity: 0.8; }"
            "QPushButton:pressed { padding-top: 2px; }"
        ).arg(imgPath));

        return btn;
    };

    // --- 第一行：变倍 ---
    funcLayout->addWidget(createLabel("变倍"), 0, 0);
    QPushButton *btnZoomIn = createFuncBtn(true);
    QPushButton *btnZoomOut = createFuncBtn(false);
    connect(btnZoomIn, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomInClicked);
    connect(btnZoomOut, &QPushButton::clicked, this, &CPTZControlWidget::onBtnZoomOutClicked);
    funcLayout->addWidget(btnZoomIn, 0, 1);
    funcLayout->addWidget(btnZoomOut, 0, 2);

    // --- 第二行：聚焦 ---
    funcLayout->addWidget(createLabel("聚焦"), 1, 0);
    QPushButton *btnFocusNear = createFuncBtn(true);
    QPushButton *btnFocusFar = createFuncBtn(false);
    connect(btnFocusNear, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusNearClicked);
    connect(btnFocusFar, &QPushButton::clicked, this, &CPTZControlWidget::onBtnFocusFarClicked);
    funcLayout->addWidget(btnFocusNear, 1, 1);
    funcLayout->addWidget(btnFocusFar, 1, 2);

    // --- 第三行：光圈 ---
    funcLayout->addWidget(createLabel("光圈"), 2, 0);
    QPushButton *btnApertureOpen = createFuncBtn(true);
    QPushButton *btnApertureClose = createFuncBtn(false);
    connect(btnApertureOpen, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureOpenClicked);
    connect(btnApertureClose, &QPushButton::clicked, this, &CPTZControlWidget::onBtnApertureCloseClicked);
    funcLayout->addWidget(btnApertureOpen, 2, 1);
    funcLayout->addWidget(btnApertureClose, 2, 2);

    // --- 第四行：步长 ---
    funcLayout->addWidget(createLabel("步长"), 3, 0);

    QSlider *stepSlider = new QSlider(Qt::Horizontal, this);
    stepSlider->setRange(1, 10);
    stepSlider->setValue(5);
    // 简单的滑块样式
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

    // 将功能区加入主布局
    mainLayout->addWidget(functionArea, 3); // 伸缩因子3
}

// ... 下面是槽函数的空实现，具体逻辑需您根据SDK填充 ...

void CPTZControlWidget::onBtnUpClicked() { qDebug() << "Up"; }
void CPTZControlWidget::onBtnDownClicked() { qDebug() << "Down"; }
void CPTZControlWidget::onBtnLeftClicked() { qDebug() << "Left"; }
void CPTZControlWidget::onBtnRightClicked() { qDebug() << "Right"; }
void CPTZControlWidget::onBtnUpLeftClicked() { qDebug() << "UpLeft"; }
void CPTZControlWidget::onBtnUpRightClicked() { qDebug() << "UpRight"; }
void CPTZControlWidget::onBtnDownLeftClicked() { qDebug() << "DownLeft"; }
void CPTZControlWidget::onBtnDownRightClicked() { qDebug() << "DownRight"; }
void CPTZControlWidget::onBtnCenterClicked() { qDebug() << "Center/Stop"; }

void CPTZControlWidget::onBtnZoomInClicked() { qDebug() << "Zoom In"; }
void CPTZControlWidget::onBtnZoomOutClicked() { qDebug() << "Zoom Out"; }
void CPTZControlWidget::onBtnFocusNearClicked() { qDebug() << "Focus Near"; }
void CPTZControlWidget::onBtnFocusFarClicked() { qDebug() << "Focus Far"; }
void CPTZControlWidget::onBtnApertureOpenClicked() { qDebug() << "Aperture Open"; }
void CPTZControlWidget::onBtnApertureCloseClicked() { qDebug() << "Aperture Close"; }
void CPTZControlWidget::onStepChanged(int value) { qDebug() << "Step:" << value; }
