#ifndef CDEVICELISTWIDGET_H
#define CDEVICELISTWIDGET_H

#include <QWidget>

class QTreeWidget;
class QLineEdit;
class QTreeWidgetItem; // 【新增】
class CDeviceListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CDeviceListWidget(QWidget *parent = nullptr);
signals:
    // 【新增】切换码流信号，channel=0为主码流，1为子码流
    void sigSwitchStream(int channel);
private slots:
    // 【新增】处理树形控件双击
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
private:
    void setupUi();
    void initData(); // 初始化模拟数据

    QLineEdit *m_searchEdit;
    QTreeWidget *m_treeWidget;
};

#endif // CDEVICELISTWIDGET_H
