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
#include "csystemsettingswidget.h"
#include <QNetworkAccessManager>
#include <QTimer>

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

class CWindowInfoWidget;
class CDeviceListWidget;
class CPTZControlWidget;

class MainWidget : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
signals:
    void sigLogout(); // 【新增】轉發退出信號給 main.cpp

private slots:
    void getOneFrame(QImage image);

    // 业务逻辑槽函数
    void startPlayLogic(int channel = 0); // 启动实时监控逻辑
    void flipLogic();      // 翻转逻辑
    void onVideoPlaybackLogic(); // 视频回放逻辑
    void onStreamSwitchRequest(int channel);

    // 页面切换槽函数
    void onSwitchToMonitor();
    void onSwitchToLogQuery();
    void onSwitchToSystemSettings();
    void onSwitchToPlayback();

    void onOpenRegisterDialog();
    void onConfigCallback();

    // 【核心】获取设备信息槽函数
    void onFetchDeviceInfo();
    void onDeviceInfoReceived();

private:
    Ui::MainWidget *ui;
    unique_ptr<FFmpegKits> _ffmpegKits;

    void detectMotion(const QImage& currentImage);

    // 状态
    PLAYER_STATE _kPlayState;
    bool _hFlip;
    bool _vFlip;

    // 【新增】标记设备是否离线/重启中
    bool m_isDeviceOffline;

    // UI 组件指针
    CTopMenuBar* m_pTopMenuBar = nullptr;
    QWidget* m_pLeftPanel = nullptr;
    CWindowInfoWidget* m_pWindowInfoWidget = nullptr;
    CAlarmWidget* m_pAlarmWidget = nullptr;
    QWidget* m_pVideoArea = nullptr;
    CDeviceListWidget* m_pDeviceListWidget = nullptr;
    CPTZControlWidget* m_pPTZControlWidget = nullptr;
    QImage _image;

    QStackedWidget* m_pStackedWidget = nullptr;
    QWidget* m_pMonitorPage = nullptr;
    CLogQueryWidget* m_pLogQueryPage = nullptr;
    CSystemSettingsWidget* m_pSystemSettingsPage = nullptr;

    QImage m_lastFrame;
    qint64 m_lastAlarmTime = 0;

    void initLayout();
    bool eventFilter(QObject *watched, QEvent *event) override;

    NotificationServer *m_server;
    FaceApiManager *m_faceManager;
    QNetworkAccessManager *m_netManager;
    QTimer *m_infoTimer;
    QString m_osdInfoText;
};
#endif // MAINWIDGET_H
