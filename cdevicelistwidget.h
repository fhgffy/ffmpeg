#ifndef CDEVICELISTWIDGET_H
#define CDEVICELISTWIDGET_H

#include <QWidget>

class QTreeWidget;
class QLineEdit;

class CDeviceListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CDeviceListWidget(QWidget *parent = nullptr);

private:
    void setupUi();
    void initData(); // 初始化模拟数据

    QLineEdit *m_searchEdit;
    QTreeWidget *m_treeWidget;
};

#endif // CDEVICELISTWIDGET_H
