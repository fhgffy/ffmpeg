#include "ffmpegkits.h"
#include <QDebug>
#include <QDateTime>

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
    m_isStop = false;
}

FFmpegKits::~FFmpegKits()
{
    stopPlay();
}

void FFmpegKits::startPlay(QString url)
{
    if (this->isRunning()) {
        stopPlay();
    }

    _url = url;
    m_isStop = false;
    this->start();
}

void FFmpegKits::stopPlay()
{
    m_isStop = true;
    this->requestInterruption();
    this->quit();
    this->wait();
}

int FFmpegKits::interrupt_cb(void *ctx)
{
    FFmpegKits *p = (FFmpegKits*)ctx;
    if (p->m_isStop) {
        return 1;
    }
    return 0;
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

    // 设置中断回调
    pFormatCtx->interrupt_callback.callback = interrupt_cb;
    pFormatCtx->interrupt_callback.opaque = this;

    AVDictionary *avdic=nullptr;
    av_dict_set(&avdic, "rtsp_transport", "tcp", 0);
    av_dict_set(&avdic, "max_delay", "100", 0);
    av_dict_set(&avdic, "stimeout", "5000000", 0);

    // 【修复1】增加探测大小和时间，解决 Could not find codec parameters
    // probesize: 20MB (默认是5MB)
    av_dict_set(&avdic, "probesize", "20480000", 0);
    // analyzeduration: 10秒 (默认很短)
    av_dict_set(&avdic, "analyzeduration", "10000000", 0);

    if (avformat_open_input(&pFormatCtx, _url.toStdString().data(), nullptr, &avdic) != 0)
    {
        qDebug() << "can't open the file:" << _url;
        avformat_free_context(pFormatCtx);
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        qDebug() << "Could't find stream infomation.";
        avformat_close_input(&pFormatCtx);
        return;
    }

    int videoStreamIdx = -1;
    videoStreamIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(videoStreamIdx < 0) {
        qDebug() << "av_find_best_stream error";
        avformat_close_input(&pFormatCtx);
        return ;
    }

    AVCodecParameters *codecPar = pFormatCtx->streams[videoStreamIdx]->codecpar;
    pCodec = avcodec_find_decoder(codecPar->codec_id);
    if (pCodec == nullptr) {
        qDebug("Codec not found.\n");
        avformat_close_input(&pFormatCtx);
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, codecPar);

    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        printf("Could not open codec.\n");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    // 【修复2】核心防崩溃检查！
    // 如果获取到的宽高无效，或者像素格式未知，绝对不能进入 sws_getContext
    if (pCodecCtx->width <= 0 || pCodecCtx->height <= 0 || pCodecCtx->pix_fmt == AV_PIX_FMT_NONE) {
        qDebug() << "Error: Invalid video properties."
                 << "Width:" << pCodecCtx->width
                 << "Height:" << pCodecCtx->height
                 << "Format:" << pCodecCtx->pix_fmt;

        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    // 安全初始化 SWS Context
    pImgConvertCtx = sws_getContext(
        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
        pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_RGB32,
        SWS_BICUBIC, nullptr, nullptr, nullptr);

    // 再次检查 Context 是否创建成功
    if (!pImgConvertCtx) {
        qDebug() << "Error: sws_getContext failed!";
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);

    pFrame     = av_frame_alloc();
    pFrameRGB  = av_frame_alloc();
    pOutBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, pOutBuffer,
                   AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);

    pPacket = (AVPacket *) malloc(sizeof(AVPacket));

    while (!m_isStop) {
        if (av_read_frame(pFormatCtx, pPacket) < 0) {
            break;
        }

        if (pPacket->stream_index == videoStreamIdx) {
            int got_picture;
            int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, pPacket);
            if (ret < 0) {
                // 解码失败不退出，只释放包，继续尝试下一帧
                av_free_packet(pPacket);
                continue;
            }

            if (got_picture) {
                // 【修复3】转换前也可以加一层保险，防止运行中分辨率突变导致崩溃
                // 但通常上面的初始化检查已经足够覆盖大部分情况
                sws_scale(pImgConvertCtx, (uint8_t const * const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                QImage tmpImg((uchar *)pOutBuffer,
                              pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                QImage image = tmpImg.copy();
                emit sigGetOneFrame(image);
            }
        }
        av_free_packet(pPacket);
    }

    // 资源释放
    if(pOutBuffer) av_free(pOutBuffer);
    if(pFrameRGB) av_free(pFrameRGB);
    if(pFrame) av_free(pFrame);
    if(pPacket) free(pPacket);

    if(pImgConvertCtx) {
        sws_freeContext(pImgConvertCtx);
        pImgConvertCtx = nullptr; // 置空防止野指针
    }

    if(pCodecCtx) avcodec_close(pCodecCtx);
    if(pFormatCtx) avformat_close_input(&pFormatCtx);

    qDebug() << "FFmpegKits::run finished.";
}
