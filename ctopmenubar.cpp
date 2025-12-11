#include "ctopmenubar.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QEvent> // 需要包含 QEvent 头文件

CTopMenuBar::CTopMenuBar(QWidget* parent)
    : QWidget(parent)
    , m_bDragging(false)
{
    setAttribute(Qt::WA_StyledBackground);
    setMouseTracking(true);
    initUI();

    // --- 新增代码开始 ---
    // 监听父窗口事件，确保宽度同步
    if (parent) {
        parent->installEventFilter(this);
        this->setFixedWidth(parent->width()); // 初始化时同步一次
    }
    // --- 新增代码结束 ---

    m_pTimerUpdate = new QTimer(this);
    connect(m_pTimerUpdate, &QTimer::timeout, this, [this]() {
        QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
        m_pLCDNumber->display(timeStr);
    });
    m_pTimerUpdate->start(1000);
}

CTopMenuBar::~CTopMenuBar()
{
}

// --- 新增函数实现 ---
bool CTopMenuBar::eventFilter(QObject *obj, QEvent *event)
{
    // 如果是父窗口发出的 Resize 事件
    if (obj == parent() && event->type() == QEvent::Resize) {
        // 强制将自身宽度设置为父窗口宽度
        this->setFixedWidth(parentWidget()->width());
        // 或者使用 resize(parentWidget()->width(), this->height());
        return true; // 事件已处理（可选，通常返回false让父窗口继续处理）
    }
    return QWidget::eventFilter(obj, event);
}
// --------------------

void CTopMenuBar::initUI()
{
    setFixedHeight(60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet("QWidget { background-color: rgb(40, 40, 40); }");
    setContentsMargins(0, 0, 0, 0);

    auto* pMainLayout = new QHBoxLayout(this);
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->setSpacing(6);

    // 左侧：Logo + 标题
    m_pLogoLabel = new QLabel(this);
    m_pLogoLabel->setFixedSize(50, 50);
    m_pLogoLabel->setPixmap(QPixmap(":/logo/logo.png")
                                .scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pMainLayout->addWidget(m_pLogoLabel, 0);
    pMainLayout->addSpacing(6);

    m_pTitleLabel = new QLabel(u8"视频监控管理平台试用版", this);
    m_pTitleLabel->setStyleSheet("QLabel { color: white; font-size: 18px; font-weight: bold; }");
    m_pTitleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    pMainLayout->addWidget(m_pTitleLabel, 0);

    // 中间：菜单容器
    auto* menuContainer = new QWidget(this);
    auto* menuLayout = new QHBoxLayout(menuContainer);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(5);

    const QString btnStyle = "QPushButton {"
                             "background-color: rgb(55, 85, 120);"
                             "color: white;"
                             "border: none;"
                             "border-radius: 4px;"
                             "padding: 5px 8px;"
                             "text-align: center;"
                             "font-size: 14px;"
                             "}"
                             "QPushButton:hover { background-color: rgb(75, 105, 140); }"
                             "QPushButton:pressed { background-color: rgb(45, 75, 110); }";

    m_pVideoMonitorBtn   = createMenuButton(u8"视频监控",   ":/ToolBar/video_monitor.png");
    m_pVideoPlaybackBtn  = createMenuButton(u8"视频回放",   ":/ToolBar/video_playback.png");
    m_pFaceRegisterBtn   = createMenuButton(u8"人脸注册",   ":/ToolBar/map.png"); // 文字改了，图标没动
    m_pLogQueryBtn       = createMenuButton(u8"日志查询",   ":/ToolBar/log.png");
    m_pSystemSettingsBtn = createMenuButton(u8"系统设置",   ":/ToolBar/system.png");

    for (auto* btn : { m_pVideoMonitorBtn, m_pVideoPlaybackBtn, m_pFaceRegisterBtn,
                       m_pLogQueryBtn, m_pSystemSettingsBtn }) {
        btn->setStyleSheet(btnStyle);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setMinimumSize(60, 48);
        btn->setMaximumSize(QWIDGETSIZE_MAX, 48);
        menuLayout->addWidget(btn);
    }

    menuContainer->setLayout(menuLayout);
    menuContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pMainLayout->addWidget(menuContainer, 4);

    // 右侧：时间与窗口按钮
    m_pLCDNumber = new QLCDNumber(this);
    m_pLCDNumber->setFixedSize(120, 40);
    m_pLCDNumber->setDigitCount(8);
    m_pLCDNumber->setSegmentStyle(QLCDNumber::Flat);
    m_pLCDNumber->setStyleSheet("QLCDNumber { background-color: rgb(30, 30, 30); "
                                "color: rgb(0, 255, 0); border: 1px solid rgb(60, 60, 60); "
                                "border-radius: 3px; }");
    pMainLayout->addWidget(m_pLCDNumber, 0);
    pMainLayout->addSpacing(4);

    const QString windowBtnStyle = "QPushButton {"
                                   "background-color: transparent;"
                                   "color: white;"
                                   "border: none;"
                                   "font-size: 20px;"
                                   "font-weight: bold;"
                                   "}"
                                   "QPushButton:hover { background-color: rgb(60, 60, 60); border-radius: 3px; }";

    m_pMinBtn   = new QPushButton("-", this);
    m_pMaxBtn   = new QPushButton("□", this);
    m_pCloseBtn = new QPushButton("×", this);

    m_pMinBtn->setStyleSheet(windowBtnStyle);
    m_pMaxBtn->setStyleSheet(windowBtnStyle);
    m_pCloseBtn->setStyleSheet(windowBtnStyle + "QPushButton:hover { background-color: rgb(200, 0, 0); border-radius: 3px; }");

    m_pMinBtn->setFixedSize(40, 40);
    m_pMaxBtn->setFixedSize(40, 40);
    m_pCloseBtn->setFixedSize(40, 40);

    pMainLayout->addWidget(m_pMinBtn, 0);
    pMainLayout->addWidget(m_pMaxBtn, 0);
    pMainLayout->addWidget(m_pCloseBtn, 0);

    // 连接信号
    connect(m_pVideoMonitorBtn, &QPushButton::clicked, this, &CTopMenuBar::sig_VideoMonitor);
    connect(m_pVideoPlaybackBtn, &QPushButton::clicked, this, &CTopMenuBar::sig_VideoPlayback);
    connect(m_pFaceRegisterBtn, &QPushButton::clicked, this, &CTopMenuBar::sig_FaceRegister); // 关联新信号
    connect(m_pLogQueryBtn, &QPushButton::clicked, this, &CTopMenuBar::sig_LogQuery);
    connect(m_pSystemSettingsBtn, &QPushButton::clicked, this, &CTopMenuBar::sig_SystemSettings);

    connect(m_pMinBtn, &QPushButton::clicked, [this]() {
        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_pLCDNumber->display(QString::number(timestamp).right(8));
        qDebug() << "Minimize clicked at:" << timestamp;
        window()->showMinimized();
    });

    connect(m_pMaxBtn, &QPushButton::clicked, [this]() {
        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_pLCDNumber->display(QString::number(timestamp).right(8));
        if (window()->isMaximized()) {
            qDebug() << "Restore clicked at:" << timestamp;
            window()->showNormal();
        } else {
            qDebug() << "Maximize clicked at:" << timestamp;
            window()->showMaximized();
        }
    });

    connect(m_pCloseBtn, &QPushButton::clicked, [this]() {
        qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_pLCDNumber->display(QString::number(timestamp).right(8));
        qDebug() << "Close clicked at:" << timestamp;
        window()->close();
    });
}

QPushButton* CTopMenuBar::createMenuButton(const QString& text, const QString& iconPath)
{
    QPushButton* btn = new QPushButton(this);

    QVBoxLayout* layout = new QVBoxLayout(btn);
    layout->setContentsMargins(6, 3, 6, 3);
    layout->setSpacing(2);

    QLabel* iconLabel = new QLabel(btn);
    iconLabel->setFixedSize(28, 28);
    iconLabel->setPixmap(QPixmap(iconPath).scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* textLabel = new QLabel(text, btn);
    textLabel->setStyleSheet("QLabel { color: white; font-size: 11px; background: transparent; }");
    textLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(textLabel, 0, Qt::AlignCenter);

    btn->setLayout(layout);

    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setMinimumSize(65, 48);
    btn->setMaximumSize(100, 48);

    return btn;
}

void CTopMenuBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QWidget* child = childAt(event->pos());
        if (child && (qobject_cast<QPushButton*>(child) || qobject_cast<QLabel*>(child)))
        {
            QWidget::mousePressEvent(event);
            return;
        }

        m_bDragging = true;
        m_dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void CTopMenuBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDragging && (event->buttons() & Qt::LeftButton))
    {
        window()->move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void CTopMenuBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bDragging = false;
        event->accept();
    }
}
