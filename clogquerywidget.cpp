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
#include <QCoreApplication>
#include <QSqlRecord>

CLogQueryWidget::CLogQueryWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    initDatabase();
    onQueryClicked(); // 默认加载第一页数据
}

CLogQueryWidget::~CLogQueryWidget()
{
    if(m_db.isOpen()) {
        m_db.close();
    }
}

void CLogQueryWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    // --- 1. 顶部查询区域 ---
    QHBoxLayout *queryLayout = new QHBoxLayout();
    queryLayout->setSpacing(10);

    QString inputStyle = "background-color: #3e3e3e; color: white; border: 1px solid #505050; border-radius: 2px; padding: 4px;";
    QString btnStyle = "QPushButton { background-color: #0078d7; color: white; border-radius: 4px; padding: 5px 15px; font-weight: bold; }"
                       "QPushButton:hover { background-color: #008cfa; }"
                       "QPushButton:pressed { background-color: #0063b1; }";
    QString labelStyle = "color: #cccccc;";

    QLabel *lblStart = new QLabel("开始时间:", this);
    lblStart->setStyleSheet(labelStyle);
    m_startDateEdit = new QDateEdit(QDate::currentDate().addDays(-7), this); // 默认查最近7天
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setStyleSheet(inputStyle);
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd");

    QLabel *lblEnd = new QLabel("结束时间:", this);
    lblEnd->setStyleSheet(labelStyle);
    m_endDateEdit = new QDateEdit(QDate::currentDate(), this);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setStyleSheet(inputStyle);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");

    QLabel *lblType = new QLabel("日志类型:", this);
    lblType->setStyleSheet(labelStyle);
    m_typeCombo = new QComboBox(this);
    // 这里定义的类型要和 addLog 里的对应
    m_typeCombo->addItems({"全部", "系统启动", "页面跳转", "系统设置", "报警事件", "云台控制"});
    m_typeCombo->setStyleSheet(inputStyle);

    m_keywordEdit = new QLineEdit(this);
    m_keywordEdit->setPlaceholderText("内容关键字...");
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
    queryLayout->addStretch();

    mainLayout->addLayout(queryLayout);

    // --- 2. 中间表格区域 ---
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({"时间", "用户", "类型", "详情"});
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    // 深色表格样式
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
    QString pageBtnStyle = "QPushButton { background-color: #3e3e3e; color: white; border: 1px solid #505050; border-radius: 4px; padding: 4px 10px; }"
                           "QPushButton:hover { background-color: #505050; }";

    QPushButton *btnPrev = new QPushButton("上一页", this);
    btnPrev->setStyleSheet(pageBtnStyle);
    connect(btnPrev, &QPushButton::clicked, this, &CLogQueryWidget::onPrevPage);

    QPushButton *btnNext = new QPushButton("下一页", this);
    btnNext->setStyleSheet(pageBtnStyle);
    connect(btnNext, &QPushButton::clicked, this, &CLogQueryWidget::onNextPage);

    m_pageLabel = new QLabel("0 / 0 页", this);
    m_pageLabel->setStyleSheet("color: white; font-weight: bold; margin: 0 10px;");

    pageLayout->addStretch();
    pageLayout->addWidget(btnPrev);
    pageLayout->addWidget(m_pageLabel);
    pageLayout->addWidget(btnNext);
    pageLayout->addStretch();

    mainLayout->addLayout(pageLayout);
}

void CLogQueryWidget::initDatabase()
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        // 数据库存放在程序运行目录下
        m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/system_log.db");
    }

    if (!m_db.open()) {
        qDebug() << "DB Open Error:" << m_db.lastError().text();
        return;
    }

    // 创建表结构
    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS sys_logs ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "create_time TEXT, "
                  "user_name TEXT, "
                  "log_type TEXT, "
                  "content TEXT)";
    if (!query.exec(sql)) {
        qDebug() << "Create Table Error:" << query.lastError().text();
    }
}

void CLogQueryWidget::addLog(const QString &type, const QString &content, const QString &user)
{
    if (!m_db.isOpen()) return;

    QSqlQuery query;
    query.prepare("INSERT INTO sys_logs (create_time, user_name, log_type, content) "
                  "VALUES (:time, :user, :type, :content)");

    QString nowStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    query.bindValue(":time", nowStr);
    query.bindValue(":user", user);
    query.bindValue(":type", type);
    query.bindValue(":content", content);

    if (!query.exec()) {
        qDebug() << "Add Log Error:" << query.lastError().text();
    }
}

void CLogQueryWidget::refreshData()
{
    onQueryClicked();
}

void CLogQueryWidget::onQueryClicked()
{
    // 点击查询时，重置回第一页
    m_currentPage = 1;
    updateTable();
}

void CLogQueryWidget::updateTable()
{
    if (!m_db.isOpen()) return;

    // 获取查询条件
    QString startStr = m_startDateEdit->date().toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = m_endDateEdit->date().toString("yyyy-MM-dd") + " 23:59:59";
    QString type = m_typeCombo->currentText();
    QString keyword = m_keywordEdit->text().trimmed();

    // 1. 构造 WHERE 子句
    QString whereSql = " WHERE create_time BETWEEN :start AND :end";
    if (type != "全部") {
        whereSql += " AND log_type = :type";
    }
    if (!keyword.isEmpty()) {
        whereSql += " AND content LIKE :keyword";
    }

    // 2. 先查总数
    QSqlQuery queryCount;
    queryCount.prepare("SELECT COUNT(*) FROM sys_logs" + whereSql);
    queryCount.bindValue(":start", startStr);
    queryCount.bindValue(":end", endStr);
    if(type != "全部") queryCount.bindValue(":type", type);
    if(!keyword.isEmpty()) queryCount.bindValue(":keyword", "%" + keyword + "%");

    if (queryCount.exec() && queryCount.next()) {
        m_totalCount = queryCount.value(0).toInt();
    } else {
        m_totalCount = 0;
    }

    // 3. 计算分页
    m_totalPage = (m_totalCount + m_pageSize - 1) / m_pageSize;
    if (m_totalPage == 0) m_totalPage = 1;
    if (m_currentPage > m_totalPage) m_currentPage = m_totalPage;

    // 4. 查当前页数据 (倒序排列，最新的在前)
    QSqlQuery queryData;
    QString dataSql = "SELECT create_time, user_name, log_type, content FROM sys_logs"
                      + whereSql
                      + " ORDER BY create_time DESC LIMIT :limit OFFSET :offset";

    queryData.prepare(dataSql);
    queryData.bindValue(":start", startStr);
    queryData.bindValue(":end", endStr);
    if(type != "全部") queryData.bindValue(":type", type);
    if(!keyword.isEmpty()) queryData.bindValue(":keyword", "%" + keyword + "%");

    queryData.bindValue(":limit", m_pageSize);
    queryData.bindValue(":offset", (m_currentPage - 1) * m_pageSize);

    // 5. 渲染表格
    m_tableWidget->setRowCount(0);
    if (queryData.exec()) {
        while (queryData.next()) {
            int row = m_tableWidget->rowCount();
            m_tableWidget->insertRow(row);

            m_tableWidget->setItem(row, 0, new QTableWidgetItem(queryData.value(0).toString()));
            m_tableWidget->setItem(row, 1, new QTableWidgetItem(queryData.value(1).toString()));
            m_tableWidget->setItem(row, 2, new QTableWidgetItem(queryData.value(2).toString()));

            QTableWidgetItem *itemMsg = new QTableWidgetItem(queryData.value(3).toString());
            itemMsg->setToolTip(queryData.value(3).toString()); // 悬停显示全文
            m_tableWidget->setItem(row, 3, itemMsg);
        }
    }

    // 更新页码显示
    m_pageLabel->setText(QString("%1 / %2 页 (共%3条)").arg(m_currentPage).arg(m_totalPage).arg(m_totalCount));
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
