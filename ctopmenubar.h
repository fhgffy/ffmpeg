#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QLCDNumber>
#include <QMouseEvent>
#include <QTimer>

class CTopMenuBar : public QWidget
{
    Q_OBJECT

public:
    explicit CTopMenuBar(QWidget* parent = nullptr);
    ~CTopMenuBar();

signals:
    void sig_VideoMonitor();
    void sig_VideoPlayback();
    void sig_FaceRegister(); // 【修改】原 sig_ElectronicMap 改为 sig_FaceRegister
    void sig_LogQuery();
    void sig_SystemSettings();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void initUI();
    QPushButton* createMenuButton(const QString& text, const QString& iconPath);

protected:
        // 添加事件过滤器声明
        bool eventFilter(QObject *obj, QEvent *event) override;

private:

    QLabel* m_pLogoLabel = nullptr;
    QLabel* m_pTitleLabel = nullptr;
    QPushButton* m_pVideoMonitorBtn = nullptr;
    QPushButton* m_pVideoPlaybackBtn = nullptr;
    QPushButton* m_pFaceRegisterBtn = nullptr; // 【修改】变量名顺便改一下，原 m_pElectronicMapBtn
    QPushButton* m_pLogQueryBtn = nullptr;
    QPushButton* m_pSystemSettingsBtn = nullptr;
    QPushButton* m_pMinBtn = nullptr;
    QPushButton* m_pMaxBtn = nullptr;
    QPushButton* m_pCloseBtn = nullptr;
    QLCDNumber* m_pLCDNumber = nullptr;
    QTimer* m_pTimerUpdate = nullptr;//更新时间

    bool m_bDragging = false;
    QPoint m_dragPosition;
};
