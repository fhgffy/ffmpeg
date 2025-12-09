#ifndef CPTZCONTROLWIDGET_H
#define CPTZCONTROLWIDGET_H

#include <QWidget>

class CPTZControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CPTZControlWidget(QWidget *parent = nullptr);

private slots:
    // 9个方向功能槽函数
    void onBtnUpClicked();
    void onBtnDownClicked();
    void onBtnLeftClicked();
    void onBtnRightClicked();
    void onBtnUpLeftClicked();
    void onBtnUpRightClicked();
    void onBtnDownLeftClicked();
    void onBtnDownRightClicked();
    void onBtnCenterClicked();

    // 新增：云台功能槽函数
    void onBtnZoomInClicked();      // 变倍+
    void onBtnZoomOutClicked();     // 变倍-
    void onBtnFocusNearClicked();   // 聚焦+
    void onBtnFocusFarClicked();    // 聚焦-
    void onBtnApertureOpenClicked();// 光圈+
    void onBtnApertureCloseClicked();// 光圈-
    void onStepChanged(int value);  // 步长改变

private:
    void setupUi();
};

#endif // CPTZCONTROLWIDGET_H
