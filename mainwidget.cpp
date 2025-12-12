#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "cwindowinfowidget.h"
#include "cdevicelistwidget.h"
#include "cptzcontrolwidget.h"
#include "clogquerywidget.h"
#include "csystemsettingswidget.h"
#include "cryptstring.h"
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include "faceregisterdialog.h"
#include <QNetworkInterface>

MainWidget::MainWidget(QWidget *parent)
    : CFrameLessWidgetBase(parent)
    , ui(new Ui::MainWidget)
    , _ffmpegKits(new FFmpegKits)
    , _kPlayState(PLAYER_IDLE)
    , _hFlip(false)
    , _vFlip(false)
{
    ui->setupUi(this);

    // 1. 初始化布局
    initLayout();

    // 2. 连接视频解码信号
    connect(_ffmpegKits.get(), &FFmpegKits::sigGetOneFrame, this, &MainWidget::getOneFrame);

    // 3. 初始化顶部菜单栏
    m_pTopMenuBar = new CTopMenuBar(this);

    // ================= 连接菜单导航信号 =================
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoMonitor, this, &MainWidget::onSwitchToMonitor);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoPlayback, this, &MainWidget::onSwitchToPlayback);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_FaceRegister, this, &MainWidget::onOpenRegisterDialog);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_LogQuery, this, &MainWidget::onSwitchToLogQuery);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_SystemSettings, this, &MainWidget::onSwitchToSystemSettings);

    // ================= 初始化业务组件 =================

    m_server = new NotificationServer(this);
    m_server->startServer(9999);

    m_faceManager = new FaceApiManager(this);
    onConfigCallback();
    onSwitchToMonitor();

    // ================= 【日志埋点】 =================

    // 1. 系统启动
    if (m_pLogQueryPage) {
        m_pLogQueryPage->addLog("系统启动", "客户端初始化完成，进入就绪状态");
    }

    // 2. 报警回调
    connect(m_server, &NotificationServer::sigAlarmReceived, this, [this](QString type, QString content, QString time, QImage img){
        if(m_pAlarmWidget) m_pAlarmWidget->addAlarm(type, content, img);
        if(m_pLogQueryPage) m_pLogQueryPage->addLog("报警事件", QString("[%1] %2").arg(type, content));
    });

    // 3. 配置修改
    if (m_pSystemSettingsPage) {
        connect(m_pSystemSettingsPage, &CSystemSettingsWidget::sigConfigChanged, this, [this](){
            onConfigCallback();
            if(m_pLogQueryPage) m_pLogQueryPage->addLog("系统设置", "修改并保存了系统参数");
        });
    }

    // 4. 【核心修复】云台控制日志
    if (m_pPTZControlWidget) {
        // (1) 处理中心按钮翻转（特殊逻辑）
        connect(m_pPTZControlWidget, &CPTZControlWidget::sig_CenterClicked, this, [this]() {
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("执行反转操作...");
            flipLogic();
        });

        // (2) 处理所有云台操作日志 (包括上下左右、变倍等)
        connect(m_pPTZControlWidget, &CPTZControlWidget::sigMessage, this, [this](const QString &msg){
            // 左侧显示
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage(msg);
            // 写入数据库
            if(m_pLogQueryPage) m_pLogQueryPage->addLog("云台控制", msg);
        });
    }
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::initLayout()
{
    QHBoxLayout *globalLayout = new QHBoxLayout(this);
    globalLayout->setContentsMargins(0, 35, 0, 0);
    globalLayout->setSpacing(0);

    m_pStackedWidget = new QStackedWidget(this);

    // Page 0: Monitor
    m_pMonitorPage = new QWidget(this);
    QHBoxLayout *monitorLayout = new QHBoxLayout(m_pMonitorPage);
    monitorLayout->setContentsMargins(0, 0, 0, 0);
    monitorLayout->setSpacing(2);

    m_pLeftPanel = new QWidget(m_pMonitorPage);
    m_pLeftPanel->setFixedWidth(240);
    m_pLeftPanel->setStyleSheet("background-color: #2d2d2d;");
    QVBoxLayout *leftLayout = new QVBoxLayout(m_pLeftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    m_pWindowInfoWidget = new CWindowInfoWidget(m_pLeftPanel);
    m_pWindowInfoWidget->addMessage("系统初始化完成");
    m_pAlarmWidget = new CAlarmWidget(m_pLeftPanel);

    leftLayout->addWidget(m_pWindowInfoWidget, 4);
    leftLayout->addWidget(m_pAlarmWidget, 6);
    monitorLayout->addWidget(m_pLeftPanel);

    m_pVideoArea = new QWidget(m_pMonitorPage);
    m_pVideoArea->setStyleSheet("background-color: black; border: 1px solid #333;");
    m_pVideoArea->installEventFilter(this);
    monitorLayout->addWidget(m_pVideoArea, 1);

    QWidget *rightPanel = new QWidget(m_pMonitorPage);
    rightPanel->setFixedWidth(240);
    rightPanel->setStyleSheet("background-color: #2d2d2d;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_pDeviceListWidget = new CDeviceListWidget(rightPanel);
    m_pPTZControlWidget = new CPTZControlWidget(rightPanel);

    rightLayout->addWidget(m_pDeviceListWidget, 6);
    rightLayout->addWidget(m_pPTZControlWidget, 4);
    monitorLayout->addWidget(rightPanel);

    // Page 1: Log
    m_pLogQueryPage = new CLogQueryWidget(this);
    m_pLogQueryPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }");

    // Page 2: Settings
    m_pSystemSettingsPage = new CSystemSettingsWidget(this);
    m_pSystemSettingsPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }"
                                         "QGroupBox { border: 1px solid #505050; margin-top: 10px; }"
                                         "QComboBox, QLineEdit, QSpinBox { background-color: #3e3e3e; color: white; border: 1px solid #505050; }");

    m_pStackedWidget->addWidget(m_pMonitorPage);
    m_pStackedWidget->addWidget(m_pLogQueryPage);
    m_pStackedWidget->addWidget(m_pSystemSettingsPage);

    globalLayout->addWidget(m_pStackedWidget);

    if(ui->widget) {
        ui->widget->hide();
        ui->widget->setParent(nullptr);
    }
}

// ---------------------------------------------------------
// 页面切换
// ---------------------------------------------------------

void MainWidget::onSwitchToMonitor()
{
    m_pStackedWidget->setCurrentIndex(0);
    startPlayLogic();
}

void MainWidget::onSwitchToPlayback()
{
    m_pStackedWidget->setCurrentIndex(0);
    onVideoPlaybackLogic();
    if (m_pLogQueryPage) m_pLogQueryPage->addLog("页面跳转", "切换至视频回放模式");
}

void MainWidget::onSwitchToLogQuery()
{
    m_pStackedWidget->setCurrentIndex(1);
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至日志查询页面");
    // 强制刷新数据
    if (m_pLogQueryPage) m_pLogQueryPage->refreshData();
}

void MainWidget::onSwitchToSystemSettings()
{
    m_pStackedWidget->setCurrentIndex(2);
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至系统设置页面");
    if (m_pLogQueryPage) m_pLogQueryPage->addLog("页面跳转", "进入系统设置页面");
}

// ---------------------------------------------------------
// 业务逻辑
// ---------------------------------------------------------

void MainWidget::startPlayLogic()
{
    QString ip = "192.168.6.100";
    if (m_pSystemSettingsPage) {
        QString cfgIp = m_pSystemSettingsPage->getDeviceIp();
        if(!cfgIp.isEmpty()) ip = cfgIp;
    }
    QString url = QString("rtsp://admin:admin@%1/live/chn=0").arg(ip);

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("启动实时监控: " + url);

    _ffmpegKits->startPlay(url);
    _kPlayState = PLAYER_PLAYING;
}

void MainWidget::onVideoPlaybackLogic()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 startTime = now - 3600 * 1000; // 一小时前
    qint64 endTime = now;

    QString deviceIp = "192.168.6.100";
    if (m_pSystemSettingsPage) {
        QString cfgIp = m_pSystemSettingsPage->getDeviceIp();
        if(!cfgIp.isEmpty()) deviceIp = cfgIp;
    }

    QString baseUrl = QString("http://%1/xsw/api/record/hls/vod/index.m3u8").arg(deviceIp);

    KVQuery query;
    query.add("start_time", std::to_string(startTime));
    query.add("end_time", std::to_string(endTime));
    query.add("channel", "0");

    QString queryString = QString::fromStdString(query.toCrpytString());
    QString fullUrl = baseUrl + "?" + queryString;

    qDebug() << "【视频回放】请求URL:" << fullUrl;

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("请求回放录像...");

    if (_ffmpegKits) {
        _ffmpegKits->startPlay(fullUrl);
        _kPlayState = PLAYER_PLAYING;
    }
}

void MainWidget::flipLogic()
{
    _hFlip = !_hFlip;
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage(QString("图像翻转: %1").arg(_hFlip ? "开" : "关"));
    if(m_pVideoArea) m_pVideoArea->update();
}

void MainWidget::getOneFrame(QImage image)
{
    if(_kPlayState == PLAYER_PAUSE) return;
    _image = image;
    detectMotion(_image);
    if(m_pVideoArea) m_pVideoArea->update();
}

bool MainWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pVideoArea && event->type() == QEvent::Paint) {
        QPainter painter(m_pVideoArea);
        int w = m_pVideoArea->width();
        int h = m_pVideoArea->height();
        painter.fillRect(0, 0, w, h, Qt::black);

        if (!_image.isNull()) {
            QImage img = _image.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img = img.mirrored(_hFlip, _vFlip);
            int x = (w - img.width()) / 2;
            int y = (h - img.height()) / 2;
            painter.drawImage(x, y, img);
        }
        return true;
    }
    return CFrameLessWidgetBase::eventFilter(watched, event);
}

void MainWidget::detectMotion(const QImage& currentImage)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    // 1. 报警冷却时间：3秒内不重复报警 (可改)
    if (now - m_lastAlarmTime < 3000) {
        return;
    }

    if (m_lastFrame.isNull()) {
        m_lastFrame = currentImage;
        return;
    }

    // 缩放到 100x100 进行对比 (降低计算量，同时过滤高频噪点)
    // 使用 IgnoreAspectRatio 强制拉伸，保证对比区域一致
    QImage smallCur = currentImage.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage smallLast = m_lastFrame.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int changedPixelCount = 0;

    // --- 【关键参数调整区域】 ---
    // 参数1：单像素差异阈值 (1-765)
    // 越小越灵敏，但太小会把噪点当成移动。建议 30-80。
    int pixelDiffThreshold = 40;

    // 参数2：报警触发阈值 (1-10000)
    // 画面中有多少个点变了才算报警？
    // 100x100总共10000个点。设置 50 表示 0.5% 的区域变动就报警。
    int alarmTriggerCount = 50;
    // -------------------------

    // 2. 逐像素对比 (x++, y++ 全扫描)
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            QColor c1 = smallCur.pixelColor(x, y);
            QColor c2 = smallLast.pixelColor(x, y);

            // 计算 RGB 三通道的总差异
            int diff = qAbs(c1.red() - c2.red()) +
                       qAbs(c1.green() - c2.green()) +
                       qAbs(c1.blue() - c2.blue());

            if (diff > pixelDiffThreshold) {
                changedPixelCount++;
            }
        }
    }

    // 更新上一帧
    m_lastFrame = currentImage;

    // 3. 判断是否触发报警
    if (changedPixelCount > alarmTriggerCount) {
        qDebug() << ">>> 移动侦测触发！变动像素点数:" << changedPixelCount;

        // 触发后的操作
        if (m_pAlarmWidget) {
            // 抓拍当前原始画面
            QImage snapshot = currentImage.copy();
            m_pAlarmWidget->addAlarm("移动侦测", QString("动态像素:%1").arg(changedPixelCount), snapshot);
        }

        // 写入日志 (确保你已集成了日志模块)
        if (m_pLogQueryPage) {
            m_pLogQueryPage->addLog("报警事件", QString("检测到画面移动，变动幅度:%1").arg(changedPixelCount));
        }

        m_lastAlarmTime = now;
    }
}

void MainWidget::onOpenRegisterDialog()
{
    FaceRegisterDialog dlg(this);
    dlg.exec();
}

void MainWidget::onConfigCallback()
{
    if (!m_faceManager || !m_pSystemSettingsPage) return;

    QString deviceIp = m_pSystemSettingsPage->getDeviceIp();
    QString deviceUser = m_pSystemSettingsPage->getDeviceUser();
    QString devicePwd = m_pSystemSettingsPage->getDevicePwd();
    QString localIp = m_pSystemSettingsPage->getLocalIp();
    int localPort = m_pSystemSettingsPage->getLocalPort();

    if (deviceIp.isEmpty()) deviceIp = "192.168.6.100";
    if (deviceUser.isEmpty()) deviceUser = "admin";
    if (devicePwd.isEmpty()) devicePwd = "admin";
    if (localIp.isEmpty()) localIp = "127.0.0.1";

    m_faceManager->setCallback(deviceIp, deviceUser, devicePwd, localIp, localPort);
}
