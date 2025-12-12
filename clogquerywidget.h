#ifndef CLOGQUERYWIDGET_H
#define CLOGQUERYWIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

class QTableWidget;
class QDateEdit;
class QLineEdit;
class QComboBox;
class QLabel;

class CLogQueryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CLogQueryWidget(QWidget *parent = nullptr);
    ~CLogQueryWidget();

    // 【核心接口】外部调用此函数添加日志
    void addLog(const QString &type, const QString &content, const QString &user = "admin");

    // 【核心接口】刷新数据（供 MainWidget 切换页面时调用）
    void refreshData();

private slots:
    void onQueryClicked();    // 查询按钮点击
    void onPrevPage();        // 上一页
    void onNextPage();        // 下一页

private:
    void setupUi();
    void initDatabase();      // 初始化数据库
    void updateTable();       // 执行查询并刷新表格

private:
    // UI控件
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QComboBox *m_typeCombo;
    QLineEdit *m_keywordEdit;
    QTableWidget *m_tableWidget;
    QLabel *m_pageLabel;

    // 数据库与分页变量
    QSqlDatabase m_db;
    int m_currentPage = 1;
    int m_pageSize = 15;      // 每页显示15条
    int m_totalCount = 0;     // 总记录数
    int m_totalPage = 1;      // 总页数
};

#endif // CLOGQUERYWIDGET_H
