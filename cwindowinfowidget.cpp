#include "cwindowinfowidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateTime>

CWindowInfoWidget::CWindowInfoWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void CWindowInfoWidget::setupUi()
{
    // 这里使用布局是为了管理内部控件，不影响外部 MainWidget 的无布局模式
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏
    QLabel *titleLabel = new QLabel(tr("窗口信息"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("background-color: #2d2d2d; color: white; padding: 6px; font-weight: bold; border-bottom: 1px solid #505050;");
    titleLabel->setFixedHeight(30);
    layout->addWidget(titleLabel);

    // 表格
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << tr("时间") << tr("消息"));
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setAlternatingRowColors(true);

    // 仿截图深色样式
    m_tableWidget->setStyleSheet(
        "QTableWidget { background-color: #3e3e3e; color: #cccccc; border: none; }"
        "QHeaderView::section { background-color: #505050; color: white; border: none; padding: 4px; }"
        "QTableWidget::item { padding: 4px; }"
        "QTableWidget::item:selected { background-color: #0078d7; }"
    );

    layout->addWidget(m_tableWidget);
}

void CWindowInfoWidget::addMessage(const QString &msg)
{
    int row = m_tableWidget->rowCount();
    m_tableWidget->insertRow(row);
    QString timeStr = QDateTime::currentDateTime().toString("HH:mm:ss");

    QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_tableWidget->setItem(row, 0, timeItem);

    QTableWidgetItem *msgItem = new QTableWidgetItem(msg);
    msgItem->setToolTip(msg);
    m_tableWidget->setItem(row, 1, msgItem);

    m_tableWidget->scrollToBottom();
}
