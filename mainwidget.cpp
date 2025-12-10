#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "cwindowinfowidget.h"
#include "cdevicelistwidget.h"
#include "cptzcontrolwidget.h"
#include "clogquerywidget.h"  // 【必需】日志查询窗口头文件
#include "cryptstring.h"      // 【必需】Token生成工具
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>

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

    // 点击“日志查询” -> 切换到日志页
    connect(m_pTopMenuBar, &CTopMenuBar::sig_LogQuery,
            this, &MainWidget::onSwitchToLogQuery);

    // ================= 云台反转信号 =================
    if (m_pPTZControlWidget) {
        connect(m_pPTZControlWidget, &CPTZControlWidget::sig_CenterClicked, this, [this]() {
            if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("执行反转操作...");
            flipLogic();
        });
    }

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
    //左侧
    m_pLeftPanel = new QWidget(m_pMonitorPage);
        m_pLeftPanel->setFixedWidth(240);
        m_pLeftPanel->setStyleSheet("background-color: #2d2d2d;"); // 统一左侧背景色

        QVBoxLayout *leftLayout = new QVBoxLayout(m_pLeftPanel);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(1); // 两个控件之间留1px缝隙

        // 1. 上半部分：窗口信息
        m_pWindowInfoWidget = new CWindowInfoWidget(m_pLeftPanel);
        // 注意：不再需要 setFixedWidth，由父容器 m_pLeftPanel 控制宽度
        m_pWindowInfoWidget->addMessage("系统初始化完成");

        // 2. 下半部分：图文警情 【新增】
        m_pAlarmWidget = new CAlarmWidget(m_pLeftPanel);

        // 添加到左侧布局，设置伸缩因子，例如 4:6 分配高度
        leftLayout->addWidget(m_pWindowInfoWidget, 4);
        leftLayout->addWidget(m_pAlarmWidget, 6);

        // 将左侧面板加入监控页布局
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
    // 保持深色风格
    m_pLogQueryPage->setStyleSheet("QWidget { background-color: #2d2d2d; color: #cccccc; }");

    // ==================================================
    // 添加到堆栈
    // ==================================================
    m_pStackedWidget->addWidget(m_pMonitorPage);  // Index 0
    m_pStackedWidget->addWidget(m_pLogQueryPage); // Index 1

    // 将堆栈窗口添加到主布局
    globalLayout->addWidget(m_pStackedWidget);

    // 隐藏默认的 ui->widget
    if(ui->widget) {
        ui->widget->hide();
        ui->widget->setParent(nullptr);
    }
    // 模拟每隔 5-10 秒触发一次报警
    m_pAlarmTimer = new QTimer(this);
    connect(m_pAlarmTimer, &QTimer::timeout, this, &MainWidget::onSimulateAlarm);
    m_pAlarmTimer->start(5000); // 5秒触发一次抓拍
}

// ---------------------------------------------------------
// 页面切换槽函数
// ---------------------------------------------------------

// 切换到：视频监控 (实时流)
void MainWidget::onSwitchToMonitor()
{
    // 1. 切到监控页
    m_pStackedWidget->setCurrentIndex(0);

    // 2. 播放实时 RTSP 流
    startPlayLogic();
}

// 切换到：监控回放 (HLS流)
void MainWidget::onSwitchToPlayback()
{
    // 1. 切到监控页 (回放也是在视频窗口看)
    m_pStackedWidget->setCurrentIndex(0);

    // 2. 执行回放逻辑
    onVideoPlaybackLogic();
}

// 切换到：日志查询
void MainWidget::onSwitchToLogQuery()
{
    // 1. 切到日志页
    m_pStackedWidget->setCurrentIndex(1);

    if(m_pWindowInfoWidget) m_pWindowInfoWidget->addMessage("切换至日志查询页面");
}

// ---------------------------------------------------------
// 业务逻辑函数
// ---------------------------------------------------------

// 实时监控逻辑
void MainWidget::startPlayLogic()
{
    // IP 需要根据你的实际环境修改，这里保持和云台一致
    QString url = "rtsp://admin:admin@192.168.6.100/live/chn=0";

    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage("启动实时监控: " + url);

    // FFmpegKits 内部会自动处理停止旧线程
    _ffmpegKits->startPlay(url);
    _kPlayState = PLAYER_PLAYING;
}

// 视频回放逻辑 (生成 Token 并播放)
void MainWidget::onVideoPlaybackLogic()
{
    // 1. 设置时间范围 (例如：过去1小时)
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 startTime = now - 3600 * 1000;
    qint64 endTime = now;

    // 【修正】IP 修改为 192.168.6.100 (与云台控制成功的 IP 一致)
    QString deviceIp = "192.168.6.100";

    // 尝试使用加密接口 /xsw/
    QString baseUrl = QString("http://%1/xsw/api/record/hls/vod/index.m3u8").arg(deviceIp);

    // 2. 构建参数
    KVQuery query;
    query.add("start_time", std::to_string(startTime));
    query.add("end_time", std::to_string(endTime));
    query.add("channel", "0");

    // 3. 生成带签名 Token 的参数字符串
    QString queryString = QString::fromStdString(query.toCrpytString());

    // 4. 拼接完整 URL 【关键定义】
    QString fullUrl = baseUrl + "?" + queryString;

    qDebug() << "【视频回放】请求URL:" << fullUrl;

    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage("请求回放录像...");

    // 5. 启动播放
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

    // 触发重绘
    if(m_pVideoArea) m_pVideoArea->update();
}

void MainWidget::getOneFrame(QImage image)
{
    if(_kPlayState == PLAYER_PAUSE) return;

    _image = image;
    // 刷新视频区域
    if(m_pVideoArea) m_pVideoArea->update();
}

bool MainWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 只处理视频区域的绘图事件
    if (watched == m_pVideoArea && event->type() == QEvent::Paint)
    {
        QPainter painter(m_pVideoArea);
        int w = m_pVideoArea->width();
        int h = m_pVideoArea->height();

        // 黑色背景
        painter.fillRect(0, 0, w, h, Qt::black);

        if (!_image.isNull())
        {
            // 保持比例缩放
            QImage img = _image.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img = img.mirrored(_hFlip, _vFlip); // 处理翻转

            int x = (w - img.width()) / 2;
            int y = (h - img.height()) / 2;
            painter.drawImage(x, y, img);
        }
        return true;
    }
    return CFrameLessWidgetBase::eventFilter(watched, event);
}

void MainWidget::onSimulateAlarm()
{
    // 1. 检查当前是否有视频画面
    if (_image.isNull()) {
        return; // 如果没视频，就不抓拍
    }

    // 2. 抓拍当前画面 (深拷贝一份，防止多线程冲突)
    QImage snapshot = _image.copy();

    // 3. 缩放一下图片，作为缩略图存储（可选，为了列表显示不占太大内存）
    // snapshot = snapshot.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 4. 随机模拟一个报警类型 (实际项目中这里应该是根据协议判断)
    QString type;
    QString content;
    if (qrand() % 2 == 0) {
        type = "移动侦测";
        content = "画面有变动";
    } else {
        type = "人脸识别";
        content = "识别到陌生人";
    }

    // 5. 将抓拍到的真实图片传给左下角的控件
    if (m_pAlarmWidget) {
        m_pAlarmWidget->addAlarm(type, content, snapshot);
    }
}
