#ifndef CONVENTION_NAMETAG_VIDEODECODER_HPP
#define CONVENTION_NAMETAG_VIDEODECODER_HPP

#include "decoder.hpp"

// extern C required
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <chrono>
#include <filesystem>
#include <string>

class VideoDecoder : public Decoder {
  public:
    explicit VideoDecoder(const std::filesystem::path &file, int width, int height);
    ~VideoDecoder() override;

    void DecodeFrame(uint8_t *outBuffer, int bufferSize) override;

  private:
    void Replay();

    AVFormatContext *_formatContext{};
    int _streamIndex;
    AVCodecParameters *_codecParameters;
    AVCodecContext *_codecContext;
    struct SwsContext *_swsContext;
    AVStream *_videoStream;

    AVFrame *_rgbFrameBuffer{av_frame_alloc()};

    const int _outWidth;
    const int _outHeight;

    std::chrono::time_point<std::chrono::steady_clock> _startTime;
};

#endif
