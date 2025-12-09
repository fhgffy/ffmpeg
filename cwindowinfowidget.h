#ifndef CWINDOWINFOWIDGET_H
#define CWINDOWINFOWIDGET_H

#include <QWidget>

class QTableWidget;

class CWindowInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CWindowInfoWidget(QWidget *parent = nullptr);
    void addMessage(const QString &msg);

private:
    void setupUi();
    QTableWidget *m_tableWidget;
};

#endif // CWINDOWINFOWIDGET_H
