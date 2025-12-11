#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "cwindowinfowidget.h"
#include "cdevicelistwidget.h"
#include "cptzcontrolwidget.h"
#include "clogquerywidget.h"
#include "csystemsettingswidget.h" // 【必需】系统设置
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

    // 1. 初始化多页面布局 (使用 QStackedWidget)
    initLayout();

    // 2. 连接视频解码信号
    connect(_ffmpegKits.get(), &FFmpegKits::sigGetOneFrame, this, &MainWidget::getOneFrame);

    // 3. 初始化顶部菜单栏
    m_pTopMenuBar = new CTopMenuBar(this);

    // ================= 连接菜单导航信号 =================
    // 点击“视频监控” -> 切换到监控页并播放实时流
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoMonitor,
            this, &MainWidget::onSwitchToMonitor);

    // 点击“视频回放” -> 切换到监控页并播放录像流
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoPlayback,
            this, &MainWidget::onSwitchToPlayback);

    // 点击 "人脸注册" -> 打开注册弹窗
    connect(m_pTopMenuBar, &CTopMenuBar::sig_FaceRegister,
            this, &MainWidget::onOpenRegisterDialog);

    // 点击“日志查询” -> 切换到日志页
    connect(m_pTopMenuBar, &CTopMenuBar::sig_LogQuery,
            this, &MainWidget::onSwitchToLogQuery);

    // 【新增】点击“系统设置” -> 切换到设置页
    connect(m_pTopMenuBar, &CTopMenuBar::sig_SystemSettings,
            this, &MainWidget::onSwitchToSystemSettings);

    // ================= 云台反转信号 =================
    if (m_pPTZControlWidget) {
        connect(m_pPTZControlWidget, &CPTZControlWidget::sig_CenterClicked, this, [this]() {
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("执行反转操作...");
            flipLogic();
        });
    }

    // 1. 启动报警回调服务器 (监听 9999 端口)
    m_server = new NotificationServer(this);
    m_server->startServer(9999);

    // 2. 连接报警信号 -> 更新左下角图文警情
    connect(m_server, &NotificationServer::sigAlarmReceived, this, [this](QString type, QString content, QString time, QImage img){
        if(m_pAlarmWidget) {
            m_pAlarmWidget->addAlarm(type, content, img);
        }
    });

    // 3. 初始化 API 管理器
    m_faceManager = new FaceApiManager(this);

    // 【新增】连接系统设置页面的“配置修改”信号
    // 当用户在设置页点击保存时，自动重新配置回调地址
    if (m_pSystemSettingsPage) {
        connect(m_pSystemSettingsPage, &CSystemSettingsWidget::sigConfigChanged,
                this, &MainWidget::onConfigCallback);
    }

    // 4. 应用初始配置 (从文件加载)
    onConfigCallback();

    // 默认启动进入视频监控模式
    onSwitchToMonitor();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::initLayout()
{
    // 全局布局
    QHBoxLayout *globalLayout = new QHBoxLayout(this);
    // 顶部留出 35px 给自定义标题栏，避免遮挡
    globalLayout->setContentsMargins(0, 35, 0, 0);
    globalLayout->setSpacing(0);

    // 创建堆栈窗口，用于管理多页面
    m_pStackedWidget = new QStackedWidget(this);

    // ==================================================
    // Page 0: 视频监控页面 (Monitor Page)
    // ==================================================
    m_pMonitorPage = new QWidget(this);
    QHBoxLayout *monitorLayout = new QHBoxLayout(m_pMonitorPage);
    monitorLayout->setContentsMargins(0, 0, 0, 0);
    monitorLayout->setSpacing(2);

    // [左侧] 面板
    m_pLeftPanel = new QWidget(m_pMonitorPage);
    m_pLeftPanel->setFixedWidth(240);
    m_pLeftPanel->setStyleSheet("background-color: #2d2d2d;");

    QVBoxLayout *leftLayout = new QVBoxLayout(m_pLeftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(1);

    // 1. 上半部分：窗口信息
    m_pWindowInfoWidget = new CWindowInfoWidget(m_pLeftPanel);
    m_pWindowInfoWidget->addMessage("系统初始化完成");

    // 2. 下半部分：图文警情
    m_pAlarmWidget = new CAlarmWidget(m_pLeftPanel);

    leftLayout->addWidget(m_pWindowInfoWidget, 4);
    leftLayout->addWidget(m_pAlarmWidget, 6);
    monitorLayout->addWidget(m_pLeftPanel);

    // [中间] 视频区域
    m_pVideoArea = new QWidget(m_pMonitorPage);
    m_pVideoArea->setStyleSheet("background-color: black; border: 1px solid #333;");
    m_pVideoArea->installEventFilter(this); // 安装绘图过滤器
    monitorLayout->addWidget(m_pVideoArea, 1);

    // [右侧] 设备列表 + 云台控制
    QWidget *rightPanel = new QWidget(m_pMonitorPage);
    rightPanel->setFixedWidth(240);
    rightPanel->setStyleSheet("background-color: #2d2d2d;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_pDeviceListWidget = new CDeviceListWidget(rightPanel);
    m_pPTZControlWidget = new CPTZControlWidget(rightPanel);

    rightLayout->addWidget(m_pDeviceListWidget, 6);
    rightLayout->addWidget(m_pPTZControlWidget, 4);
    monitorLayout->addWidget(rightPanel);

    // ==================================================
    // Page 1: 日志查询页面 (Log Page)
    // ==================================================
    m_pLogQueryPage = new CLogQueryWidget(this);
    m_pLogQueryPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }");

    // ==================================================
    // Page 2: 系统设置页面 (Settings Page) 【新增】
    // ==================================================
    m_pSystemSettingsPage = new CSystemSettingsWidget(this);
    // 保持深色风格，与整体一致
    m_pSystemSettingsPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }"
                                         "QGroupBox { border: 1px solid #505050; margin-top: 10px; }"
                                         "QComboBox, QLineEdit, QSpinBox { background-color: #3e3e3e; color: white; border: 1px solid #505050; }");

    // ==================================================
    // 添加到堆栈
    // ==================================================
    m_pStackedWidget->addWidget(m_pMonitorPage);      // Index 0
    m_pStackedWidget->addWidget(m_pLogQueryPage);     // Index 1
    m_pStackedWidget->addWidget(m_pSystemSettingsPage); // Index 2

    // 将堆栈窗口添加到主布局
    globalLayout->addWidget(m_pStackedWidget);

    // 隐藏默认的 ui->widget
    if(ui->widget) {
        ui->widget->hide();
        ui->widget->setParent(nullptr);
    }
}

// ---------------------------------------------------------
// 页面切换槽函数
// ---------------------------------------------------------

// 切换到：视频监控 (Index 0)
void MainWidget::onSwitchToMonitor()
{
    m_pStackedWidget->setCurrentIndex(0);
    startPlayLogic(); // 播放实时流
}

// 切换到：监控回放 (Index 0)
void MainWidget::onSwitchToPlayback()
{
    // 回放也是在视频窗口看 (Index 0)，只是播放源变了
    m_pStackedWidget->setCurrentIndex(0);
    onVideoPlaybackLogic();
}

// 切换到：日志查询 (Index 1)
void MainWidget::onSwitchToLogQuery()
{
    m_pStackedWidget->setCurrentIndex(1);
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至日志查询页面");
}

// 【新增】切换到：系统设置 (Index 2)
void MainWidget::onSwitchToSystemSettings()
{
    m_pStackedWidget->setCurrentIndex(2);
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至系统设置页面");
}

// ---------------------------------------------------------
// 业务逻辑函数
// ---------------------------------------------------------

void MainWidget::startPlayLogic()
{
    // 【修改】优先从系统配置获取 IP，如果没有则用默认
    QString ip = "192.168.6.100";
    if (m_pSystemSettingsPage) {
        QString cfgIp = m_pSystemSettingsPage->getDeviceIp();
        if(!cfgIp.isEmpty()) ip = cfgIp;
    }

    QString url = QString("rtsp://admin:admin@%1/live/chn=0").arg(ip);

    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage("启动实时监控: " + url);

    _ffmpegKits->startPlay(url);
    _kPlayState = PLAYER_PLAYING;
}

void MainWidget::onVideoPlaybackLogic()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 startTime = now - 3600 * 1000;
    qint64 endTime = now;

    // 【修改】从配置获取 IP
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

    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage("请求回放录像...");

    if (_ffmpegKits) {
        _ffmpegKits->startPlay(fullUrl);
        _kPlayState = PLAYER_PLAYING;
    }
}

void MainWidget::flipLogic()
{
    _hFlip = !_hFlip;
    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage(QString("图像翻转: %1").arg(_hFlip ? "开" : "关"));

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
    if (watched == m_pVideoArea && event->type() == QEvent::Paint)
    {
        QPainter painter(m_pVideoArea);
        int w = m_pVideoArea->width();
        int h = m_pVideoArea->height();

        painter.fillRect(0, 0, w, h, Qt::black);

        if (!_image.isNull())
        {
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
    if (now - m_lastAlarmTime < 1000) {
        return;
    }

    if (m_lastFrame.isNull()) {
        m_lastFrame = currentImage;
        return;
    }

    QImage smallCur = currentImage.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage smallLast = m_lastFrame.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int changedPixelCount = 0;
    int sensitivity = 30;

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            QColor c1 = smallCur.pixelColor(x, y);
            QColor c2 = smallLast.pixelColor(x, y);

            int diff = qAbs(c1.red() - c2.red()) +
                       qAbs(c1.green() - c2.green()) +
                       qAbs(c1.blue() - c2.blue());

            if (diff > sensitivity) {
                changedPixelCount++;
            }
        }
    }

    m_lastFrame = currentImage;

    if (changedPixelCount > 15) {
        qDebug() << ">>> 触发报警！抓拍！";
        if (m_pAlarmWidget) {
            QImage snapshot = currentImage.copy();
            m_pAlarmWidget->addAlarm("移动侦测", "画面检测到异动", snapshot);
        }
        m_lastAlarmTime = now;
    }
}

void MainWidget::onOpenRegisterDialog()
{
    FaceRegisterDialog dlg(this);
    dlg.exec();
}

// 【新增实现】配置报警回调
// 逻辑：优先使用用户在系统设置里填写的参数，如果没填再自动获取
void MainWidget::onConfigCallback()
{
    if (!m_faceManager) return;
    if (!m_pSystemSettingsPage) return;

    // 1. 尝试从配置页面获取参数
    QString deviceIp = m_pSystemSettingsPage->getDeviceIp();
    QString deviceUser = m_pSystemSettingsPage->getDeviceUser();
    QString devicePwd = m_pSystemSettingsPage->getDevicePwd();
    QString localIp = m_pSystemSettingsPage->getLocalIp();
    int localPort = m_pSystemSettingsPage->getLocalPort();

    // 2. 如果配置为空（通常是第一次运行），给一些默认值或尝试自动获取
    if (deviceIp.isEmpty()) deviceIp = "192.168.6.100";
    if (deviceUser.isEmpty()) deviceUser = "admin";
    if (devicePwd.isEmpty()) devicePwd = "admin";

    // 如果本地IP为空，自动获取一个
    if (localIp.isEmpty()) {
        const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (const QHostAddress &entry : ipAddressesList) {
            if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
                localIp = entry.toString();
                break;
            }
        }
    }

    qDebug() << "正在配置回调 => Device:" << deviceIp << " Local:" << localIp << ":" << localPort;

    // 3. 调用管理器下发配置
    m_faceManager->setCallback(deviceIp, deviceUser, devicePwd, localIp, localPort);
}

