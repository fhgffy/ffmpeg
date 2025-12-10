#ifndef CALARMWIDGET_H
#define CALARMWIDGET_H

#include <QWidget>
#include <QListWidget>

// 警情数据结构
struct AlarmEvent {
    QString time;       // 时间
    QString type;       // 类型：人脸识别/移动侦测
    QString content;    // 描述内容
    QString status;     // 状态：待处理/已处理
    QImage snapshot;    // 抓拍图
};

class CAlarmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CAlarmWidget(QWidget *parent = nullptr);

    // 添加一条警情
    void addAlarm(const QString &type, const QString &content, const QImage &img);

private:
    void setupUi();
    void initMockData(); // 初始化模拟数据

    QListWidget *m_listWidget;
};

#endif // CALARMWIDGET_H
