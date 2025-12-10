#ifndef CLOGQUERYWIDGET_H
#define CLOGQUERYWIDGET_H

#include <QWidget>
#include <QList>
#include <QDateTime>

class QTableWidget;
class QDateEdit;
class QLineEdit;
class QComboBox;
class QLabel;

// 模拟日志数据结构
struct LogData {
    QString time;
    QString user;
    QString type;
    QString content;
    QString ip;
};

class CLogQueryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CLogQueryWidget(QWidget *parent = nullptr);

private slots:
    void onQueryClicked();    // 查询按钮点击
    void onPrevPage();        // 上一页
    void onNextPage();        // 下一页

private:
    void setupUi();
    void initMockData();      // 初始化模拟数据
    void updateTable();       // 刷新表格显示

private:
    // UI控件
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QComboBox *m_typeCombo;
    QLineEdit *m_keywordEdit;
    QTableWidget *m_tableWidget;
    QLabel *m_pageLabel;

    // 数据与分页
    QList<LogData> m_allLogs;       // 所有原始数据
    QList<LogData> m_filteredLogs;  // 查询过滤后的数据
    int m_currentPage = 1;
    int m_pageSize = 15;            // 每页显示条数
    int m_totalPage = 1;
};

#endif // CLOGQUERYWIDGET_H
