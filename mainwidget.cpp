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
#include <QSplitter>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QAuthenticator>

MainWidget::MainWidget(QWidget *parent)
    : CFrameLessWidgetBase(parent)
    , ui(new Ui::MainWidget)
    , _ffmpegKits(new FFmpegKits)
    , _kPlayState(PLAYER_IDLE)
    , _hFlip(false)
    , _vFlip(false)
    , m_isDeviceOffline(false)
{
    ui->setupUi(this);
    initLayout();

    // 连接视频解码信号
    connect(_ffmpegKits.get(), &FFmpegKits::sigGetOneFrame, this, &MainWidget::getOneFrame);

    // 初始化顶部菜单栏
    m_pTopMenuBar = new CTopMenuBar(this);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoMonitor, this, &MainWidget::onSwitchToMonitor);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoPlayback, this, &MainWidget::onSwitchToPlayback);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_FaceRegister, this, &MainWidget::onOpenRegisterDialog);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_LogQuery, this, &MainWidget::onSwitchToLogQuery);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_SystemSettings, this, &MainWidget::onSwitchToSystemSettings);

    // 初始化业务组件
    m_server = new NotificationServer(this);
    m_server->startServer(9999);

    m_faceManager = new FaceApiManager(this);
    onConfigCallback();

    // 初始化网络管理器
    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, &QNetworkAccessManager::authenticationRequired,
            this, [this](QNetworkReply *, QAuthenticator *authenticator){
        QString user = "admin";
        QString pwd = "admin";
        if (m_pSystemSettingsPage) {
            QString cfgUser = m_pSystemSettingsPage->getDeviceUser();
            QString cfgPwd = m_pSystemSettingsPage->getDevicePwd();
            if(!cfgUser.isEmpty()) user = cfgUser;
            if(!cfgPwd.isEmpty()) pwd = cfgPwd;
        }
        authenticator->setUser(user);
        authenticator->setPassword(pwd);
    });

    // 定时获取设备信息 (3秒一次)
    m_infoTimer = new QTimer(this);
    connect(m_infoTimer, &QTimer::timeout, this, &MainWidget::onFetchDeviceInfo);
    m_infoTimer->start(3000);

    onSwitchToMonitor();

    // 日志与报警
    if (m_pLogQueryPage) {
        m_pLogQueryPage->addLog("系统启动", "客户端初始化完成，进入就绪状态");
    }

    connect(m_server, &NotificationServer::sigAlarmReceived, this, [this](QString type, QString content, QString time, QImage img){
        if(m_pAlarmWidget) m_pAlarmWidget->addAlarm(type, content, img);
        if(m_pLogQueryPage) m_pLogQueryPage->addLog("报警事件", QString("[%1] %2").arg(type, content));
    });

    // 系统设置保存后的回调
    if (m_pSystemSettingsPage) {
        connect(m_pSystemSettingsPage, &CSystemSettingsWidget::sigConfigChanged, this, [this](){
            onConfigCallback();

            // 立即标记为离线
            m_isDeviceOffline = true;
            // 立即清空当前画面
            _image = QImage();
            // 停止播放器
            _ffmpegKits->stopPlay();
            // 强制刷新界面显示重启中
            if(m_pVideoArea) m_pVideoArea->update();

            if(m_pLogQueryPage) m_pLogQueryPage->addLog("系统设置", "修改配置，设备可能正在重启...");
        });
    }

    if (m_pPTZControlWidget) {
        connect(m_pPTZControlWidget, &CPTZControlWidget::sig_CenterClicked, this, [this]() {
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("执行反转操作...");
            flipLogic();
        });
        connect(m_pPTZControlWidget, &CPTZControlWidget::sigMessage, this, [this](const QString &msg){
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage(msg);
            if(m_pLogQueryPage) m_pLogQueryPage->addLog("云台控制", msg);
        });
    }

    if (m_pDeviceListWidget) {
        connect(m_pDeviceListWidget, &CDeviceListWidget::sigSwitchStream,
                this, &MainWidget::onStreamSwitchRequest);
    }

    startPlayLogic(0);
}

MainWidget::~MainWidget() { delete ui; }

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
    monitorLayout->setSpacing(0);

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, m_pMonitorPage);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #404040; border: 1px solid #505050; width: 4px; }");

    // Left
    m_pLeftPanel = new QWidget(mainSplitter);
    m_pLeftPanel->setMinimumWidth(150);
    m_pLeftPanel->setStyleSheet("background-color: #2d2d2d;");
    QVBoxLayout *leftLayout = new QVBoxLayout(m_pLeftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    m_pWindowInfoWidget = new CWindowInfoWidget(m_pLeftPanel);
    m_pWindowInfoWidget->addMessage("系统初始化完成");
    m_pAlarmWidget = new CAlarmWidget(m_pLeftPanel);
    leftLayout->addWidget(m_pWindowInfoWidget, 4);
    leftLayout->addWidget(m_pAlarmWidget, 6);

    // Center
    m_pVideoArea = new QWidget(mainSplitter);
    m_pVideoArea->setStyleSheet("background-color: black; border: 1px solid #333;");
    m_pVideoArea->installEventFilter(this);
    m_pVideoArea->setMinimumWidth(400);

    // Right
    QWidget *rightPanel = new QWidget(mainSplitter);
    rightPanel->setFixedWidth(240);
    rightPanel->setStyleSheet("background-color: #2d2d2d;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    m_pDeviceListWidget = new CDeviceListWidget(rightPanel);
    m_pPTZControlWidget = new CPTZControlWidget(rightPanel);
    rightLayout->addWidget(m_pDeviceListWidget, 6);
    rightLayout->addWidget(m_pPTZControlWidget, 4);

    mainSplitter->addWidget(m_pLeftPanel);
    mainSplitter->addWidget(m_pVideoArea);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes(QList<int>() << 240 << 1000 << 240);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setStretchFactor(2, 0);
    monitorLayout->addWidget(mainSplitter);

    // Page 1 & 2
    m_pLogQueryPage = new CLogQueryWidget(this);
    m_pLogQueryPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }");
    m_pSystemSettingsPage = new CSystemSettingsWidget(this);
    m_pSystemSettingsPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }"
                                         "QGroupBox { border: 1px solid #505050; margin-top: 10px; }"
                                         "QComboBox, QLineEdit, QSpinBox { background-color: #3e3e3e; color: white; border: 1px solid #505050; }");

    m_pStackedWidget->addWidget(m_pMonitorPage);
    m_pStackedWidget->addWidget(m_pLogQueryPage);
    m_pStackedWidget->addWidget(m_pSystemSettingsPage);
    globalLayout->addWidget(m_pStackedWidget);

    if(ui->widget) { ui->widget->hide(); ui->widget->setParent(nullptr); }
}

void MainWidget::onSwitchToMonitor()
{
    m_pStackedWidget->setCurrentIndex(0);

    // 【核心修复】
    // 去掉了之前的 `if (!_ffmpegKits->isRunning())` 判断。
    // 只要点击监控按钮且设备在线，就强制调用 startPlayLogic(0)。
    // startPlayLogic 内部会自动停止当前正在播放的流（无论是回放还是之前的监控），然后重新连接实时流。
    if (!m_isDeviceOffline) {
        startPlayLogic(0);
    } else {
        if(m_pVideoArea) m_pVideoArea->update();
    }

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至视频监控页面");
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
    if (m_pLogQueryPage) m_pLogQueryPage->refreshData();
}

void MainWidget::onSwitchToSystemSettings()
{
    m_pStackedWidget->setCurrentIndex(2);
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至系统设置页面");
    if (m_pLogQueryPage) m_pLogQueryPage->addLog("页面跳转", "进入系统设置页面");
}

void MainWidget::onStreamSwitchRequest(int channel)
{
    QString type = (channel == 0) ? "主码流" : "子码流";
    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("正在切换至: " + type);
    startPlayLogic(channel);
}

void MainWidget::startPlayLogic(int channel)
{
    QString ip = "192.168.6.100";
    if (m_pSystemSettingsPage) {
        QString cfgIp = m_pSystemSettingsPage->getDeviceIp();
        if(!cfgIp.isEmpty()) ip = cfgIp;
    }
    QString url = QString("rtsp://admin:admin@%1/live/chn=%2").arg(ip).arg(channel);
    qDebug() << ">>> 启动播放 URL:" << url;

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage(QString("连接流媒体: chn=%1").arg(channel));

    // 启动前先清空旧图
    _image = QImage();

    // 强制启动新播放，底层会自动停止旧的
    _ffmpegKits->startPlay(url);
    _kPlayState = PLAYER_PLAYING;

    if(m_pVideoArea) m_pVideoArea->update();
}

void MainWidget::onVideoPlaybackLogic()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 startTime = now - 3600 * 1000;
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
    QString fullUrl = baseUrl + "?" + QString::fromStdString(query.toCrpytString());

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("请求回放录像...");
    if (_ffmpegKits) {
        _image = QImage(); // 清空画面
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

        // 状态1：设备离线/重启中
        if (m_isDeviceOffline) {
            painter.setPen(Qt::white);
            painter.setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
            painter.drawText(m_pVideoArea->rect(), Qt::AlignCenter, "摄像头正在重启中...");
        }
        // 状态2：设备在线，但还没有视频画面 (正在连接RTSP)
        else if (_image.isNull()) {
            painter.setPen(Qt::white);
            painter.setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
            painter.drawText(m_pVideoArea->rect(), Qt::AlignCenter, "设备已上线，正在连接视频...");
        }
        // 状态3：正常显示视频
        else {
            QImage img = _image.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img = img.mirrored(_hFlip, _vFlip);
            int x = (w - img.width()) / 2;
            int y = (h - img.height()) / 2;
            painter.drawImage(x, y, img);

            if(!m_osdInfoText.isEmpty()) {
                painter.setPen(Qt::white);
                painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
                QFontMetrics fm(painter.font());
                int textWidth = fm.horizontalAdvance(m_osdInfoText);
                int textHeight = fm.height();
                int textX = w - textWidth - 10;
                int textY = h - 10;
                painter.fillRect(textX - 5, textY - textHeight + 2, textWidth + 10, textHeight + 2, QColor(0, 0, 0, 150));
                painter.drawText(textX, textY, m_osdInfoText);
            }
        }
        return true;
    }
    return CFrameLessWidgetBase::eventFilter(watched, event);
}

void MainWidget::detectMotion(const QImage& currentImage)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastAlarmTime < 3000) return;
    if (m_lastFrame.isNull()) { m_lastFrame = currentImage; return; }

    QImage smallCur = currentImage.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage smallLast = m_lastFrame.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    int changedPixelCount = 0;
    int pixelDiffThreshold = 40;
    int alarmTriggerCount = 50;

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            QColor c1 = smallCur.pixelColor(x, y);
            QColor c2 = smallLast.pixelColor(x, y);
            int diff = qAbs(c1.red() - c2.red()) + qAbs(c1.green() - c2.green()) + qAbs(c1.blue() - c2.blue());
            if (diff > pixelDiffThreshold) changedPixelCount++;
        }
    }
    m_lastFrame = currentImage;

    if (changedPixelCount > alarmTriggerCount) {
        if (m_pAlarmWidget) {
            QImage snapshot = currentImage.copy();
            m_pAlarmWidget->addAlarm("移动侦测", QString("动态像素:%1").arg(changedPixelCount), snapshot);
        }
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

void MainWidget::onFetchDeviceInfo()
{
    QString ip = "192.168.6.100";
    if (m_pSystemSettingsPage) {
        QString cfgIp = m_pSystemSettingsPage->getDeviceIp();
        if(!cfgIp.isEmpty()) ip = cfgIp;
    }

    QString urlStr = QString("http://%1/xsw/tmpInfo").arg(ip);
    QUrl url(urlStr);

    KVQuery kv;
    QString queryString = QString::fromStdString(kv.toCrpytString());
    url.setQuery(queryString);

    QNetworkRequest request(url);
    QNetworkReply *reply = m_netManager->get(request);

    QTimer *timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply](){ reply->abort(); });
    timeoutTimer->start(3000);

    connect(reply, &QNetworkReply::finished, this, &MainWidget::onDeviceInfoReceived);
}

void MainWidget::onDeviceInfoReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(!reply) return;

    if(reply->error() == QNetworkReply::NoError) {
        bool wasOffline = m_isDeviceOffline;
        m_isDeviceOffline = false;

        // 核心重连逻辑：
        // 如果之前是离线，或者当前没有播放（无论是主动停止还是异常停止）
        // 且处于监控页面，则自动重连
        if ((wasOffline || !_ffmpegKits->isRunning()) && m_pStackedWidget->currentIndex() == 0) {
             qDebug() << ">>> 检测到设备上线或流中断，尝试自动重连视频...";
             startPlayLogic(0);
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if(doc.isObject()) {
            QJsonObject obj = doc.object();
            QString totalCountStr = obj.value("totalcount").toVariant().toString();
            QString cpuStr = obj.value("cpu").toVariant().toString();
            QString diskFreeStr = obj.value("disk_free").toVariant().toString();
            double hours = totalCountStr.toDouble() / 3600.0;

            if(!totalCountStr.isEmpty()) {
                m_osdInfoText = QString("运行: %1h | CPU: %2% | 磁盘: %3MB")
                                    .arg(QString::number(hours, 'f', 1))
                                    .arg(cpuStr)
                                    .arg(diskFreeStr);
                if(m_pVideoArea) m_pVideoArea->update();
            }
        }
    } else {
        if (!m_isDeviceOffline) {
            qDebug() << ">>> 心跳丢失，设备离线/重启中...";
            m_isDeviceOffline = true;
            _image = QImage();
            m_osdInfoText.clear();

            if (_ffmpegKits->isRunning()) {
                _ffmpegKits->stopPlay();
            }
            if(m_pVideoArea) m_pVideoArea->update();
        }
    }
    reply->deleteLater();
}
