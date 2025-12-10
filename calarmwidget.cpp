#include "calarmwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QTimer>

CAlarmWidget::CAlarmWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();


}

void CAlarmWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 1. 标题栏
    QLabel *titleLabel = new QLabel("图文警情", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(30);
    titleLabel->setStyleSheet("background-color: #2d2d2d; color: white; font-weight: bold; border-bottom: 1px solid #505050;");
    layout->addWidget(titleLabel);

    // 2. 警情列表
    m_listWidget = new QListWidget(this);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setStyleSheet("QListWidget { background-color: #3e3e3e; border: none; outline: none; }"
                                "QListWidget::item { border-bottom: 1px solid #505050; }");
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    layout->addWidget(m_listWidget);
}

void CAlarmWidget::addAlarm(const QString &type, const QString &content, const QImage &img)
{
    // 创建自定义的 Item Widget
    QWidget *itemWidget = new QWidget;
    itemWidget->setFixedHeight(80); // 单行高度

    QHBoxLayout *hLayout = new QHBoxLayout(itemWidget);
    hLayout->setContentsMargins(5, 5, 5, 5);
    hLayout->setSpacing(5);

    // 左侧：文本信息布局
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0,0,0,0);

    QLabel *lblTime = new QLabel(QDateTime::currentDateTime().toString("HH:mm:ss"));
    lblTime->setStyleSheet("color: white; font-weight: bold; font-size: 12px;");

    QLabel *lblType = new QLabel(type);
    lblType->setStyleSheet("color: #cccccc; font-size: 11px;");

    QLabel *lblStatus = new QLabel("待处理");
    // 待处理显示橙色背景，圆角
    lblStatus->setStyleSheet("background-color: #E6A23C; color: white; border-radius: 3px; padding: 1px 4px; font-size: 10px;");
    lblStatus->setFixedSize(45, 18);
    lblStatus->setAlignment(Qt::AlignCenter);

    textLayout->addWidget(lblTime);
    textLayout->addWidget(lblType);
    textLayout->addWidget(lblStatus);
    textLayout->addStretch();

    // 右侧：图片
    QLabel *lblImg = new QLabel;
    lblImg->setFixedSize(90, 60);
    lblImg->setScaledContents(true);
    lblImg->setPixmap(QPixmap::fromImage(img));
    lblImg->setStyleSheet("border: 1px solid #666;");

    hLayout->addLayout(textLayout);
    hLayout->addStretch();
    hLayout->addWidget(lblImg);

    // 添加到 ListWidget
    QListWidgetItem *item = new QListWidgetItem(m_listWidget);
    item->setSizeHint(itemWidget->sizeHint());

    // 插入到第一行（最新消息在最上面）
    m_listWidget->insertItem(0, item);
    m_listWidget->setItemWidget(item, itemWidget);

    // 限制列表长度，比如最多显示50条
    if (m_listWidget->count() > 50) {
        delete m_listWidget->takeItem(m_listWidget->count() - 1);
    }
}
