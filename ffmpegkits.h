#ifndef FFMPEGKITS_H
#define FFMPEGKITS_H

#include <QThread>
#include <QImage>
#include <atomic> // 【新增】引入原子操作头文件

class FFmpegKits : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegKits(QObject *parent = nullptr);
    ~FFmpegKits();

    void startPlay(QString url);
    void stopPlay();

    // 【新增】FFmpeg 的中断回调函数，必须是 static
    static int interrupt_cb(void *ctx);

protected:
    void run() override;

signals:
    void sigGetOneFrame(QImage image);

private:
    QString _url;

    // 【修改】使用原子布尔值，确保跨线程访问安全
    std::atomic<bool> m_isStop;
};

#endif // FFMPEGKITS_H
