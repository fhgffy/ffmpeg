#include "CTitleBar.h"

#include <QHBoxLayout>
#include <windows.h>
#include <qt_windows.h>
#include <QMessageBox>

CTitleBar::CTitleBar(QWidget *parent) : QWidget(parent)
{
    initUI();
}

// 在CTitleBar.cpp文件中，修改initUI()方法：

void CTitleBar::initUI()
{
    //禁止父窗口影响子窗口样式,必须加上，否则样式不会起作用
    setAttribute(Qt::WA_StyledBackground);

    this->setFixedHeight(40);  // 调整为合适的高度
    this->setStyleSheet("background-color: transparent;");  // 透明背景
//    this->setStyleSheet("background-color: white;");  //调试背景

    // 创建按钮 - 确保每个按钮都是独立的对象
    _psetButton = new QPushButton(this);
    _psetButton->setFixedSize(36, 36);
    _psetButton->setObjectName("setButton");  // 添加对象名，方便调试
    _psetButton->setStyleSheet(
            "QPushButton{background-image:url(:/resources/titlebar/set.svg);border:none;background-repeat:no-repeat;background-position:center;}"
            "QPushButton:hover{background-color:rgba(99, 99, 99, 0.5);background-image:url(:/resources/titlebar/set_hover.svg);}");

    _pminButton = new QPushButton(this);
    _pminButton->setFixedSize(36, 36);
    _pminButton->setObjectName("minButton");
    _pminButton->setStyleSheet(
            "QPushButton{background-image:url(:/resources/titlebar/min.svg);border:none;background-repeat:no-repeat;background-position:center;}"
            "QPushButton:hover{background-color:rgba(99, 99, 99, 0.5);background-image:url(:/resources/titlebar/min_hover.svg);}");

    _pcloseButton = new QPushButton(this);
    _pcloseButton->setFixedSize(36, 36);
    _pcloseButton->setObjectName("closeButton");
    _pcloseButton->setStyleSheet(
            "QPushButton{background-image:url(:/resources/titlebar/close.svg);border:none;background-repeat:no-repeat;background-position:center;}"
            "QPushButton:hover{background-color:rgba(232, 17, 35, 0.9);background-image:url(:/resources/titlebar/close_hover.svg);}");

    // 使用水平布局
    QHBoxLayout *phLayout = new QHBoxLayout(this);
    phLayout->setContentsMargins(50, 3, 0, 3);  // 调整边距
    phLayout->setSpacing(5);  // 减小间距

    // 添加弹簧，使按钮靠右对齐
    phLayout->addStretch(999);

    phLayout->addWidget(_psetButton);
    phLayout->addWidget(_pminButton);
    phLayout->addWidget(_pcloseButton);

    // 连接信号
    connect(_pminButton, &QPushButton::clicked, this, &CTitleBar::onClickedSlot);
    connect(_pcloseButton, &QPushButton::clicked, this, &CTitleBar::onClickedSlot);

//    // 连接关闭信号
//    connect(_pcloseButton, &CTitleBar::sigClose, this, &CTitleBar::closeSlot);
}

// 修改onClickedSlot()方法，移除最大化相关逻辑：

void CTitleBar::onClickedSlot()
{
    QPushButton *pbtn = qobject_cast<QPushButton*>(sender());
    QWidget *pwindow = this->window();

    if (pbtn == _pminButton) {
        pwindow->showMinimized();
    }

    else if(pbtn == _pcloseButton) {
        emit sigClose();  // 发射信号，通知父窗口关闭
    }
}

