#include "clogquerywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QDebug>

CLogQueryWidget::CLogQueryWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    initMockData();
    onQueryClicked(); // 默认加载一次
}

void CLogQueryWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    // --- 1. 顶部查询区域 ---
    QHBoxLayout *queryLayout = new QHBoxLayout();
    queryLayout->setSpacing(10);

    // 样式通用设置
    QString inputStyle = "background-color: #3e3e3e; color: white; border: 1px solid #505050; border-radius: 2px; padding: 4px;";
    QString btnStyle = "QPushButton { background-color: #0078d7; color: white; border-radius: 4px; padding: 5px 15px; }"
                       "QPushButton:hover { background-color: #008cfa; }"
                       "QPushButton:pressed { background-color: #0063b1; }";
    QString labelStyle = "color: #cccccc;";

    QLabel *lblStart = new QLabel("开始时间:", this);
    lblStart->setStyleSheet(labelStyle);
    m_startDateEdit = new QDateEdit(QDate::currentDate().addDays(-7), this);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setStyleSheet(inputStyle);

    QLabel *lblEnd = new QLabel("结束时间:", this);
    lblEnd->setStyleSheet(labelStyle);
    m_endDateEdit = new QDateEdit(QDate::currentDate(), this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setStyleSheet(inputStyle);

    QLabel *lblType = new QLabel("日志类型:", this);
    lblType->setStyleSheet(labelStyle);
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems({"全部", "系统登录", "设备控制", "报警信息", "系统设置"});
    m_typeCombo->setStyleSheet(inputStyle);

    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText("请输入关键字...");
    m_keywordEdit->setStyleSheet(inputStyle);

    QPushButton *btnQuery = new QPushButton("查询", this);
    btnQuery->setCursor(Qt::PointingHandCursor);
    btnQuery->setStyleSheet(btnStyle);
    connect(btnQuery, &QPushButton::clicked, this, &CLogQueryWidget::onQueryClicked);

    queryLayout->addWidget(lblStart);
    queryLayout->addWidget(m_startDateEdit);
    queryLayout->addWidget(lblEnd);
    queryLayout->addWidget(m_endDateEdit);
    queryLayout->addWidget(lblType);
    queryLayout->addWidget(m_typeCombo);
    queryLayout->addWidget(m_keywordEdit);
    queryLayout->addWidget(btnQuery);
    queryLayout->addStretch(); // 弹簧

    mainLayout->addLayout(queryLayout);

    // --- 2. 中间表格区域 ---
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({"时间", "用户", "类型", "详情", "IP地址"});
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    // 表格样式
    m_tableWidget->setStyleSheet(
        "QTableWidget { background-color: #2d2d2d; color: #cccccc; gridline-color: #505050; border: 1px solid #505050; }"
        "QHeaderView::section { background-color: #3e3e3e; color: white; border: none; padding: 6px; font-weight: bold; }"
        "QTableWidget::item { padding: 4px; }"
        "QTableWidget::item:selected { background-color: #0078d7; color: white; }"
        "QTableWidget::item:alternate { background-color: #353535; }"
    );
    mainLayout->addWidget(m_tableWidget);

    // --- 3. 底部翻页区域 ---
    QHBoxLayout *pageLayout = new QHBoxLayout();

    QPushButton *btnPrev = new QPushButton("上一页", this);
    QPushButton *btnNext = new QPushButton("下一页", this);
    m_pageLabel = new QLabel("1 / 1 页", this);
    m_pageLabel->setStyleSheet("color: white; font-weight: bold; margin: 0 10px;");

    QString pageBtnStyle = "QPushButton { background-color: #3e3e3e; color: white; border: 1px solid #505050; border-radius: 4px; padding: 4px 10px; }"
                           "QPushButton:hover { background-color: #505050; }"
                           "QPushButton:pressed { background-color: #2d2d2d; }";
    btnPrev->setStyleSheet(pageBtnStyle);
    btnNext->setStyleSheet(pageBtnStyle);

    connect(btnPrev, &QPushButton::clicked, this, &CLogQueryWidget::onPrevPage);
    connect(btnNext, &QPushButton::clicked, this, &CLogQueryWidget::onNextPage);

    pageLayout->addStretch();
    pageLayout->addWidget(btnPrev);
    pageLayout->addWidget(m_pageLabel);
    pageLayout->addWidget(btnNext);
    pageLayout->addStretch();

    mainLayout->addLayout(pageLayout);
}

void CLogQueryWidget::initMockData()
{
    // 生成100条模拟数据
    m_allLogs.clear();
    QDateTime baseTime = QDateTime::currentDateTime();
    QStringList users = {"admin", "operator", "guest"};
    QStringList types = {"系统登录", "设备控制", "报警信息", "系统设置"};
    QStringList ips = {"192.168.1.10", "192.168.1.101", "127.0.0.1"};

    for (int i = 0; i < 100; ++i) {
        LogData log;
        log.time = baseTime.addSecs(-i * 3600).toString("yyyy-MM-dd HH:mm:ss");
        log.user = users[i % users.size()];
        log.type = types[i % types.size()];
        log.ip = ips[i % ips.size()];

        if (log.type == "系统登录") log.content = "用户登录成功";
        else if (log.type == "设备控制") log.content = QString("控制设备 %1 云台转动").arg(i%4);
        else if (log.type == "报警信息") log.content = "检测到移动侦测报警";
        else log.content = "修改了系统配置参数";

        m_allLogs.append(log);
    }
}

void CLogQueryWidget::onQueryClicked()
{
    QString keyword = m_keywordEdit->text().trimmed();
    QString type = m_typeCombo->currentText();

    m_filteredLogs.clear();
    for (const auto &log : m_allLogs) {
        // 简单模拟过滤逻辑
        bool matchType = (type == "全部") || (log.type == type);
        bool matchKey = keyword.isEmpty() || log.content.contains(keyword) || log.user.contains(keyword);

        if (matchType && matchKey) {
            m_filteredLogs.append(log);
        }
    }

    m_currentPage = 1;
    m_totalPage = (m_filteredLogs.size() + m_pageSize - 1) / m_pageSize;
    if (m_totalPage == 0) m_totalPage = 1;

    updateTable();
}

void CLogQueryWidget::updateTable()
{
    m_tableWidget->setRowCount(0);
    m_pageLabel->setText(QString("%1 / %2 页").arg(m_currentPage).arg(m_totalPage));

    int start = (m_currentPage - 1) * m_pageSize;
    int end = qMin(start + m_pageSize, m_filteredLogs.size());

    for (int i = start; i < end; ++i) {
        const LogData &log = m_filteredLogs[i];
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        m_tableWidget->setItem(row, 0, new QTableWidgetItem(log.time));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(log.user));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(log.type));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(log.content));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(log.ip));
    }
}

void CLogQueryWidget::onPrevPage()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        updateTable();
    }
}

void CLogQueryWidget::onNextPage()
{
    if (m_currentPage < m_totalPage) {
        m_currentPage++;
        updateTable();
    }
}
