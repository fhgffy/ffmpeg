#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "FFmpegKits.h"
#include "CFrameLessWidgetBase.h"
#include <QWidget>
#include <QImage>
#include "ctopmenubar.h"
#include <memory>

// 1. 将枚举定义移到类外面，解决编译错误
enum PLAYER_STATE
{
    PLAYER_IDLE = 0,
    PLAYER_PLAYING,
    PLAYER_PAUSE,
    PLAYER_STOP
};

using std::unique_ptr;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

// 前向声明
class CWindowInfoWidget;
class CDeviceListWidget; // 新增
class CPTZControlWidget;
class MainWidget : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    // 移除 paintEvent，我们用 eventFilter 在子控件上画图
    // void paintEvent(QPaintEvent *event) override;

private slots:
    void getOneFrame(QImage image);
    // 移除 UI 按钮的槽函数，因为按钮被删了，但保留逻辑函数
    void startPlayLogic(); // 改名：启动播放逻辑
    void flipLogic();      // 改名：翻转逻辑

private:
    Ui::MainWidget *ui;
    unique_ptr<FFmpegKits> _ffmpegKits;

    // 状态
    PLAYER_STATE _kPlayState;
    bool _hFlip;
    bool _vFlip;

    // UI 组件指针
    CTopMenuBar* m_pTopMenuBar = nullptr;
    CWindowInfoWidget* m_pWindowInfoWidget = nullptr; // 左侧
    QWidget* m_pVideoArea = nullptr;                  // 中间视频区
    CDeviceListWidget* m_pDeviceListWidget = nullptr; // 右侧设备列表
    CPTZControlWidget* m_pPTZControlWidget = nullptr; // 新增：右侧云台控制
    QImage _image;

    // 辅助
    void initLayout();
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWIDGET_H
