#include "cdevicelistwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>

CDeviceListWidget::CDeviceListWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    initData();
}

void CDeviceListWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // 1. 标题 "设备列表"
    QLabel *titleLabel = new QLabel(tr("设备列表"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(30);
    titleLabel->setStyleSheet("background-color: #2d2d2d; color: white; font-weight: bold; border-bottom: 1px solid #505050;");
    mainLayout->addWidget(titleLabel);

    // 2. 搜索栏区域
    QWidget *searchContainer = new QWidget(this);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(5, 5, 5, 5);
    searchLayout->setSpacing(2);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("搜索设备..."));
    m_searchEdit->setStyleSheet("QLineEdit { background-color: #3e3e3e; color: white; border: 1px solid #505050; border-radius: 2px; padding: 2px; }");

    QPushButton *searchBtn = new QPushButton(this);
    searchBtn->setText("🔍"); // 简单用字符代替图标
    searchBtn->setFixedSize(24, 24);
    searchBtn->setStyleSheet("QPushButton { background-color: #3e3e3e; color: white; border: none; } QPushButton:hover { background-color: #505050; }");

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(searchBtn);
    mainLayout->addWidget(searchContainer);

    // 3. 设备树
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderHidden(true); // 隐藏表头
    m_treeWidget->setIndentation(20);    // 缩进
    m_treeWidget->setAnimated(true);

    // 样式表：深色树形控件
    m_treeWidget->setStyleSheet(
        "QTreeWidget { background-color: #3e3e3e; color: #cccccc; border: none; }"
        "QTreeWidget::item { height: 25px; }"
        "QTreeWidget::item:hover { background-color: #454545; }"
        "QTreeWidget::item:selected { background-color: #0078d7; color: white; }"
    );
    mainLayout->addWidget(m_treeWidget);
}

void CDeviceListWidget::initData()
{
    // 模拟添加数据
    QTreeWidgetItem *rootGroup = new QTreeWidgetItem(m_treeWidget);
    rootGroup->setText(0, "默认分组");
    rootGroup->setExpanded(true);

    QTreeWidgetItem *netVideoGroup = new QTreeWidgetItem(rootGroup);
    netVideoGroup->setText(0, "网络视频");
    netVideoGroup->setExpanded(true);

    // 添加几个模拟摄像头
    for (int i = 1; i <= 4; ++i) {
        QTreeWidgetItem *cameraItem = new QTreeWidgetItem(netVideoGroup);
        cameraItem->setText(0, QString("网络视频%1").arg(i));

        // 给第一个摄像头加码流信息，模仿截图
        if (i == 1) {
            cameraItem->setExpanded(true);
            QTreeWidgetItem *mainStream = new QTreeWidgetItem(cameraItem);
            mainStream->setText(0, "主码流");
            // mainStream->setIcon(0, QIcon("...")); // 如果有图标可以加

            QTreeWidgetItem *subStream = new QTreeWidgetItem(cameraItem);
            subStream->setText(0, "子码流");
        }
    }
}
