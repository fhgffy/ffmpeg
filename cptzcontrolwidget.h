#ifndef CPTZCONTROLWIDGET_H
#define CPTZCONTROLWIDGET_H

#include <QWidget>

class CPTZControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CPTZControlWidget(QWidget *parent = nullptr);

private slots:
    // 9个功能槽函数
    void onBtnUpClicked();
    void onBtnDownClicked();
    void onBtnLeftClicked();
    void onBtnRightClicked();
    void onBtnUpLeftClicked();
    void onBtnUpRightClicked();
    void onBtnDownLeftClicked();
    void onBtnDownRightClicked();
    void onBtnCenterClicked();

private:
    void setupUi();
};

#endif // CPTZCONTROLWIDGET_H
