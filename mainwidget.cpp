#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QPainter>
#include <QDebug>

MainWidget::MainWidget(QWidget *parent)
    : CFrameLessWidgetBase(parent)
    , ui(new Ui::MainWidget)
    , _ffmpegKits(new FFmpegKits)
    , _kPlayState(PLAYER_IDLE)
    , _hFlip(false)
    , _vFlip(false)
{
    ui->setupUi(this);

    connect(_ffmpegKits.get(), &FFmpegKits::sigGetOneFrame, this, &MainWidget::getOneFrame);
    //菜单栏对象
    m_pTopMenuBar = new CTopMenuBar(this);
    connect(m_pTopMenuBar, &CTopMenuBar::sig_VideoMonitor,
            this, &MainWidget::on_playButton_clicked);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::getOneFrame(QImage image)
{
    ui->imageLabel->clear();
    if(_kPlayState == PLAYER_PAUSE)
    {
       return;
    }

    _image = image;
    update(); //调用update将执行paintEvent函数
}

void MainWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int showWidth = this->width() - 100;
    int showHeight = this->height() - 50;

    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, this->width(), this->height()); //先画成白色

    if (_image.size().width() <= 0)
    {
        return;
    }

    //将图像按比例缩放
    QImage img = _image.scaled(QSize(showWidth, showHeight), Qt::KeepAspectRatio);
    img = img.mirrored(_hFlip, _vFlip);

    int x = this->width() - img.width();
    int y = this->height() - img.height();
    x /= 2;
    y /= 2;
    qDebug()<< "x:" << x << ", y:" << y << endl;
    painter.drawImage(QPoint(x, 100), img); //画出图像
}


void MainWidget::on_pauseButton_clicked()
{}

void MainWidget::on_playButton_clicked()
{
    if (_kPlayState == PLAYER_IDLE) //第一次按下为启动，后续则为继续
    {
        ui->addrText->setEnabled(false);
        //QString url = ui->addrText->text();
        QString url = QString::fromUtf8("rtsp://admin:admin@192.168.6.100/live/chn=0");
        qDebug() << "url:" << url << endl;

        _ffmpegKits->startPlay(url);
        ui->addrText->setText("rtsp网络连接中...");
        _kPlayState = PLAYER_PLAYING;
    }
    else
    {
        _kPlayState = PLAYER_PAUSE;
    }
}

void MainWidget::on_flipButton_clicked()
{
    _hFlip = !_hFlip;
}

