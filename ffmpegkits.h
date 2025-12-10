#ifndef FFMPEGKITS_H
#define FFMPEGKITS_H

#include <QThread>
#include <QImage>

class FFmpegKits : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegKits(QObject *parent = nullptr);
    ~FFmpegKits(); // 记得析构函数中也要处理停止

    void startPlay(QString url);
    void stopPlay(); // 【新增】停止播放函数

protected:
    void run() override;

signals:
    void sigGetOneFrame(QImage image);

private:
    QString _url;
    bool m_isStop = false; // 【新增】停止标志
};

#endif // FFMPEGKITS_H
