#ifndef MAINWIDGET_H
#define MAINWIDGET_H
#include "FFmpegKits.h"

#include <QWidget>
#include <QImage>

#include <memory>

using std::unique_ptr;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

enum PLAYER_STATE
{
    PLAYER_IDLE = 0,
    PLAYER_PLAYING,
    PLAYER_PAUSE,
    PLAYER_STOP
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    void paintEvent(QPaintEvent *event) override;

private slots:
    void getOneFrame(QImage image);

    void on_pauseButton_clicked();

    void on_playButton_clicked();

    void on_flipButton_clicked();


private:
    Ui::MainWidget *ui;
    unique_ptr<FFmpegKits> _ffmpegKits;
    QImage _image;
    PLAYER_STATE _kPlayState;
    bool _hFlip;//水平翻转
    bool _vFlip;//垂直翻转
};
#endif // MAINWIDGET_H
