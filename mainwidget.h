#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ffmpegkits.h"
#include "cframelesswidgetbase.h"
#include <QWidget>
#include <QImage>
#include "ctopmenubar.h"
#include <memory>
#include "clogquerywidget.h"
#include <QStackedWidget>
#include "calarmwidget.h"
#include "notificationserver.h"
#include "faceapimanager.h"
#include "faceregisterdialog.h"
#include "csystemsettingswidget.h" // 【新增】包含系统设置头文件

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
class CDeviceListWidget;
class CPTZControlWidget;

class MainWidget : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void getOneFrame(QImage image);

    // 业务逻辑槽函数
    // 【修改】改为带参数，默认 -1 表示使用当前配置，0/1表示强制指定
    void startPlayLogic(int channel = 0); // 启动实时监控逻辑
    void flipLogic();      // 翻转逻辑
    void onVideoPlaybackLogic(); // 视频回放逻辑
    // 【新增】响应设备列表的切换请求
    void onStreamSwitchRequest(int channel);

    // 【修改】页面切换槽函数
    void onSwitchToMonitor();         // 切到监控/回放页 (Index 0)
    void onSwitchToLogQuery();        // 切到日志页 (Index 1)
    void onSwitchToSystemSettings();  // 【新增】切到设置页 (Index 2)
    void onSwitchToPlayback();        // 切到回放模式 (逻辑上还在 Index 0)

    void onOpenRegisterDialog();      // 打开注册窗口
    void onConfigCallback();          // 自动配置回调（响应配置修改）

private:
    Ui::MainWidget *ui;
    unique_ptr<FFmpegKits> _ffmpegKits;

    void detectMotion(const QImage& currentImage); // 移动侦测算法

    // 状态
    PLAYER_STATE _kPlayState;
    bool _hFlip;
    bool _vFlip;

    // UI 组件指针
    CTopMenuBar* m_pTopMenuBar = nullptr;

    // 左侧容器布局
    QWidget* m_pLeftPanel = nullptr;
    CWindowInfoWidget* m_pWindowInfoWidget = nullptr;
    CAlarmWidget* m_pAlarmWidget = nullptr;

    // 中间及右侧
    QWidget* m_pVideoArea = nullptr;
    CDeviceListWidget* m_pDeviceListWidget = nullptr;
    CPTZControlWidget* m_pPTZControlWidget = nullptr;
    QImage _image;

    // 多页面管理
    QStackedWidget* m_pStackedWidget = nullptr;
    QWidget* m_pMonitorPage = nullptr;          // Index 0: 监控+回放共用
    CLogQueryWidget* m_pLogQueryPage = nullptr; // Index 1: 日志查询
    CSystemSettingsWidget* m_pSystemSettingsPage = nullptr; // 【新增】Index 2: 系统设置

    QImage m_lastFrame;     // 上一帧画面用于对比
    qint64 m_lastAlarmTime = 0; // 控制报警频率

    // 辅助
    void initLayout();
    bool eventFilter(QObject *watched, QEvent *event) override;

    NotificationServer *m_server;
    FaceApiManager *m_faceManager;
};
#endif // MAINWIDGET_H
