#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ffmpegkits.h"
#include "cframelesswidgetbase.h"
#include <QWidget>
#include <QImage>
#include "ctopmenubar.h"
#include <memory>
#include "clogquerywidget.h" // 确保有这个头文件
#include <QStackedWidget>
#include "calarmwidget.h"
#include "notificationserver.h"
#include "faceapimanager.h"
#include "faceregisterdialog.h"
#include "csystemsettingswidget.h"
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
    // 【新增】视频回放逻辑槽函数
    void onVideoPlaybackLogic();
    // 新增槽函数：页面切换
        void onSwitchToMonitor();
        void onSwitchToLogQuery();
        void onSwitchToPlayback();  // 切到回放

        void onOpenRegisterDialog(); // 打开注册窗口
        void onConfigCallback();     // 自动配置回调


private:
    Ui::MainWidget *ui;
    unique_ptr<FFmpegKits> _ffmpegKits;

    void detectMotion(const QImage& currentImage); // 【新增】移动侦测算法
    // 状态
    PLAYER_STATE _kPlayState;
    bool _hFlip;
    bool _vFlip;

    // UI 组件指针
    CTopMenuBar* m_pTopMenuBar = nullptr;
    // 左侧容器布局变动
        QWidget* m_pLeftPanel = nullptr;        // 左侧面板容器
        CWindowInfoWidget* m_pWindowInfoWidget = nullptr;
        CAlarmWidget* m_pAlarmWidget = nullptr; // 【新增】图文警情控件
    QWidget* m_pVideoArea = nullptr;                  // 中间视频区
    CDeviceListWidget* m_pDeviceListWidget = nullptr; // 右侧设备列表
    CPTZControlWidget* m_pPTZControlWidget = nullptr; // 新增：右侧云台控制
    QImage _image;

    QStackedWidget* m_pStackedWidget = nullptr; // 堆栈窗体
    QWidget* m_pMonitorPage = nullptr;          // 监控页容器
    CLogQueryWidget* m_pLogQueryPage = nullptr; // 日志页

    QImage m_lastFrame;     // 【新增】保存上一帧画面用于对比
    qint64 m_lastAlarmTime = 0; // 【新增】控制报警频率，防止一秒报警25次
    // 辅助
    void initLayout();
    bool eventFilter(QObject *watched, QEvent *event) override;

    NotificationServer *m_server;
    FaceApiManager *m_faceManager;

};
#endif // MAINWIDGET_H
