#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "cwindowinfowidget.h"
#include "cdevicelistwidget.h" // 包含新头文件
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWidget::MainWidget(QWidget *parent)
    : CFrameLessWidgetBase(parent)
    , ui(new Ui::MainWidget)
    , _ffmpegKits(new FFmpegKits)
    , _kPlayState(PLAYER_IDLE)
    , _hFlip(false)
    , _vFlip(false)
{
    ui->setupUi(this);

    // 1. 初始化整体布局
    initLayout();

    connect(_ffmpegKits.get(), &FFmpegKits::sigGetOneFrame, this, &MainWidget::getOneFrame);

    // 2. 菜单栏
    m_pTopMenuBar = new CTopMenuBar(this);

    // 连接顶部菜单的"视频监控"信号来触发播放，代替原来的按钮
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoMonitor,
            this, &MainWidget::startPlayLogic);

    // 可以在这里模拟调用翻转逻辑，或者绑定快捷键
    // connect(..., ..., this, &MainWidget::flipLogic);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::initLayout()
{
    // 全局水平布局: [左侧] [中间视频] [右侧]
    QHBoxLayout *globalLayout = new QHBoxLayout(this);

    // 修改处：将顶部边距从 0 改为 35 (或菜单栏的实际高度)
    // 这样左侧信息栏、中间视频和右侧设备列表都会向下偏移，不会被菜单栏遮挡
    globalLayout->setContentsMargins(0, 35, 0, 0);

    globalLayout->setSpacing(2); // 栏目间留一点缝隙

    // --- 左侧：窗口信息 ---
    m_pWindowInfoWidget = new CWindowInfoWidget(this);
    m_pWindowInfoWidget->setFixedWidth(240);
    m_pWindowInfoWidget->addMessage("系统初始化...");
    globalLayout->addWidget(m_pWindowInfoWidget);

    // --- 中间：视频区域 ---
    m_pVideoArea = new QWidget(this);
    m_pVideoArea->setStyleSheet("background-color: black; border: 1px solid #333;");
    m_pVideoArea->installEventFilter(this); // 安装绘图过滤器
    // 设置伸缩因子为 1，让视频区占据主要空间
    globalLayout->addWidget(m_pVideoArea, 1);

    // --- 右侧：设备列表与云台 ---
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(240);
    rightPanel->setStyleSheet("background-color: #2d2d2d;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 右侧上半部分：设备列表
    m_pDeviceListWidget = new CDeviceListWidget(rightPanel);
    rightLayout->addWidget(m_pDeviceListWidget);

    // 右侧下半部分：云台控制 (暂时留白/占位，以后实现)
    QWidget *ptzPlaceholder = new QWidget(rightPanel);
    // ptzPlaceholder->setStyleSheet("background-color: #333;"); // 测试看区域
    rightLayout->addWidget(ptzPlaceholder);

    // 设置上下比例，设备列表占多一点，或者各一半
    rightLayout->setStretchFactor(m_pDeviceListWidget, 6);
    rightLayout->setStretchFactor(ptzPlaceholder, 4);

    globalLayout->addWidget(rightPanel);

    // 注意：不再添加 ui->widget 到布局中，从而达到"去掉"的效果
    if(ui->widget) {
        ui->widget->hide(); // 确保隐藏
        ui->widget->setParent(nullptr); // 从对象树移除或者单纯隐藏
    }
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
    if (watched == m_pVideoArea && event->type() == QEvent::Paint)
    {
        QPainter painter(m_pVideoArea);
        int w = m_pVideoArea->width();
        int h = m_pVideoArea->height();

        painter.fillRect(0, 0, w, h, Qt::black);

        if (!_image.isNull())
        {
            QImage img = _image.scaled(QSize(w, h), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            img = img.mirrored(_hFlip, _vFlip); // 保留翻转逻辑

            int x = (w - img.width()) / 2;
            int y = (h - img.height()) / 2;
            painter.drawImage(x, y, img);
        }
        return true;
    }
    return CFrameLessWidgetBase::eventFilter(watched, event);
}

// 原来的 on_playButton_clicked 逻辑，改为通用函数
void MainWidget::startPlayLogic()
{
    if (_kPlayState == PLAYER_IDLE)
    {
        // 硬编码地址，因为移除了输入框
        QString url = "rtsp://admin:admin@192.168.6.100/live/chn=0";

        _ffmpegKits->startPlay(url);
        _kPlayState = PLAYER_PLAYING;

        if(m_pWindowInfoWidget)
            m_pWindowInfoWidget->addMessage("开始播放: " + url);
    }
    else if (_kPlayState == PLAYER_PLAYING)
    {
        _kPlayState = PLAYER_PAUSE;
        if(m_pWindowInfoWidget)
            m_pWindowInfoWidget->addMessage("暂停播放");
    }
    else if (_kPlayState == PLAYER_PAUSE)
    {
        _kPlayState = PLAYER_PLAYING;
        if(m_pWindowInfoWidget)
            m_pWindowInfoWidget->addMessage("继续播放");
    }
}

// 原来的 on_flipButton_clicked 逻辑
void MainWidget::flipLogic()
{
    _hFlip = !_hFlip;
    if(m_pWindowInfoWidget)
        m_pWindowInfoWidget->addMessage(QString("图像翻转"));
}
