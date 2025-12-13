#ifndef CTITLEBAR_H
#define CTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class CTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CTitleBar(QWidget *parent = nullptr);

signals:
    void sigClose();  // 添加这个信号
    void sigSet(); // 【新增】点击设置按钮的信号
private:
    void initUI();
    //鼠标按住标题栏时，进行窗体拖拽
    //void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onClickedSlot();
//    void closeSlot();

private:
    QLabel * _plogoLabel;
    QLabel * _ptitleTextLabel;
    QPushButton * _psetButton;
    QPushButton * _pminButton;
    QPushButton * _pmaxButton;
    QPushButton * _pcloseButton;
};

#endif // CTITLEBAR_H
