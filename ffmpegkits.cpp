#include "ffmpegkits.h"
#include <QDebug>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
}

FFmpegKits::FFmpegKits(QObject *parent)
    : QThread(parent)
{
}

FFmpegKits::~FFmpegKits()
{
    stopPlay(); // 析构时确保线程退出
}

void FFmpegKits::startPlay(QString url)
{
    // 【关键修改】如果线程正在运行，先停止它
        if (this->isRunning()) {
            stopPlay();
        }

        _url = url;
        m_isStop = false; // 重置停止标志
        this->start();    // 启动新线程
}
// 【新增】停止播放实现
void FFmpegKits::stopPlay()
{
    m_isStop = true; // 设置停止标志
    this->requestInterruption(); // 请求中断（辅助手段）
    this->quit();
    this->wait();    // 阻塞等待线程安全退出
}

void FFmpegKits::run()
{
    qDebug() << "FFmpegKits::run start url:" << _url;
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext  *pCodecCtx = nullptr;
    AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    AVPacket *pPacket = nullptr;
    uint8_t *pOutBuffer = nullptr;
    static struct SwsContext *pImgConvertCtx = nullptr;

    avformat_network_init();

    pFormatCtx = avformat_alloc_context();

    // ... (中间的字典设置代码保持不变) ...
    AVDictionary *avdic=nullptr;
    char option_key[] = "rtsp_transport";
    char option_value[] = "udp";
    av_dict_set(&avdic, option_key, option_value, 0);
    char option_key2[] = "max_delay";
    char option_value2[] = "100";
    av_dict_set(&avdic, option_key2, option_value2, 0);
    // ...

    if (avformat_open_input(&pFormatCtx, _url.toStdString().data(), nullptr, &avdic) != 0)
    {
        qDebug() << "can't open the file:" << _url;
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        qDebug() << "Could't find stream infomation.";
        return;
    }

    int videoStreamIdx = -1;
    videoStreamIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(videoStreamIdx < 0) {
        qDebug() << "av_find_best_stream error";
        return ;
    }

    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStreamIdx]->codecpar->codec_id);
    if (pCodec == nullptr) {
        qDebug("Codec not found.\n");
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStreamIdx]->codecpar);

    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        printf("Could not open codec.\n");
        return;
    }

    pImgConvertCtx = sws_getContext(
        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
        pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_RGB32,
        SWS_BICUBIC, nullptr, nullptr, nullptr);

    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);

    pFrame     = av_frame_alloc();
    pFrameRGB  = av_frame_alloc();
    pOutBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, pOutBuffer,
                   AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);

    pPacket = (AVPacket *) malloc(sizeof(AVPacket));

    // 【关键修改】循环条件增加 !m_isStop
    while (!m_isStop) {
        if (av_read_frame(pFormatCtx, pPacket) < 0) {
            break;
        }

        if (pPacket->stream_index == videoStreamIdx) {
            int got_picture;
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,pPacket);
            if (ret < 0) {
                printf("decode error.\n");
                // 不要直接return， break出去清理资源
                av_free_packet(pPacket);
                break;
            }

            if (got_picture) {
                sws_scale(pImgConvertCtx, (uint8_t const * const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                QImage tmpImg((uchar *)pOutBuffer,
                              pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                QImage image = tmpImg.copy();
                emit sigGetOneFrame(image);
            }
        }
        av_free_packet(pPacket);

        // 【建议】加上这个，让线程有机会响应中断请求
        if(QThread::currentThread()->isInterruptionRequested()) {
            m_isStop = true;
        }
    }

    // 资源释放
    av_free(pOutBuffer);
    av_free(pFrameRGB);
    av_free(pFrame);      // 记得释放 pFrame
    // av_free(pPacket);  // pPacket 是 malloc 的，最后应该 free(pPacket) 或者 av_packet_free
    if(pPacket) free(pPacket);

    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    qDebug() << "FFmpegKits::run finished.";
}
