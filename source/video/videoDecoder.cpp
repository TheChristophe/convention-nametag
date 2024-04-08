#include "videoDecoder.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <thread>

VideoDecoder::VideoDecoder(const std::filesystem::path &file, int width, int height)
    : _formatContext{avformat_alloc_context()}, _outWidth{width}, _outHeight{height} {
    auto filename = std::string(file);
    if (avformat_open_input(&_formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        throw std::runtime_error("failed to open video file!");
    }

    av_dump_format(_formatContext, 0, filename.c_str(), 0);

    for (int i = 0; i < static_cast<int>(_formatContext->nb_streams); i++) {
        if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _videoStream = _formatContext->streams[i];
            _streamIndex = i;
            _codecParameters = _videoStream->codecpar;
            break;
        }
    }
    if (_codecParameters == nullptr) {
        throw std::runtime_error("failed to find video stream!");
    }

    const AVCodec *codec = nullptr;
    // raspberry pi hardware accelerated
    if (_codecParameters->codec_id == AV_CODEC_ID_H264) {
        codec = avcodec_find_decoder_by_name("h264_omx");
    }

    // no specialized decoder, attempt automatic choice
    if (codec == nullptr) {
        codec = avcodec_find_decoder(_codecParameters->codec_id);

        // still nothing found
        if (codec == nullptr) {
            throw std::runtime_error("failed to find decoder!");
        }
    }

    _codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(_codecContext, _codecParameters);
    if (avcodec_open2(_codecContext, codec, nullptr) < 0) {
        throw std::runtime_error("failed to open codec!");
    }

    avformat_find_stream_info(_formatContext, nullptr);

    // AV_PIX_FMT_GRAY8 = Y component of YUV
    _swsContext = sws_getContext(_codecParameters->width, _codecParameters->height,
        static_cast<AVPixelFormat>(_codecParameters->format), _outWidth, _outHeight, AV_PIX_FMT_GRAY8, SWS_BILINEAR,
        nullptr, nullptr, nullptr);

    av_image_alloc(_rgbFrameBuffer->data, _rgbFrameBuffer->linesize, _outWidth, _outHeight, AV_PIX_FMT_GRAY8, 1);

    _startTime = std::chrono::steady_clock::now();
}

VideoDecoder::~VideoDecoder() {
    av_freep(&_rgbFrameBuffer->data[0]);
    av_frame_unref(_rgbFrameBuffer);
    // avcodec_close(_codecContext);
    avformat_close_input(&_formatContext);
    avformat_free_context(_formatContext);
}

void VideoDecoder::DecodeFrame(uint8_t *outBuffer, int bufferSize) {
    assert(bufferSize >= av_image_get_buffer_size(AV_PIX_FMT_GRAY8, 256, 64, 1));

    AVFrame *frame = av_frame_alloc();
    AVPacket packet;

    while (true) {
        int ret = av_read_frame(_formatContext, &packet);
        if (ret < 0) {
            std::cerr << "Error or EOF when reading frame (" << ret << ")" << std::endl;
            Replay();
            continue;
        }
        if (packet.stream_index == _streamIndex) {
            ret = avcodec_send_packet(_codecContext, &packet);
            if (ret < 0) {
                if (ret == AVERROR(EAGAIN)) {
                    std::cerr << "Previous packet not finished" << std::endl;
                } else if (ret == AVERROR_EOF) {
                    std::cerr << "Packet at EOF" << std::endl;
                } else if (ret == AVERROR(EINVAL)) {
                    std::cerr << "Massive fail when sending packet" << std::endl;
                } else if (ret == AVERROR(ENOMEM)) {
                    std::cerr << "Ran out of memory" << std::endl;
                } else {
                    std::cerr << "Unknown packet error (" << ret << ")" << std::endl;
                }
                std::exit(-1);
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(_codecContext, frame);
                if (ret < 0) {
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        std::cerr << "Empty packet?" << std::endl;
                        break;
                    } else if (ret == AVERROR(EINVAL)) {
                        std::cerr << "Massive fail when decoding this frame" << std::endl;
                    } else if (ret < 0) {
                        std::cerr << "Unknown frame error (" << ret << ")" << std::endl;
                    }
                    std::exit(-1);
                }
                /*
                  std::cout << "frame pts: " << frame->pts << std::endl;
                  std::cout << "      best effort: " <<
                  frame->best_effort_timestamp << std::endl;
                 */

                // wait until the right moment
                // TODO: wait elsewhere
                auto nextFrameTime = frame->best_effort_timestamp * av_q2d(_videoStream->time_base) * 1000.;
                std::this_thread::sleep_until(_startTime + std::chrono::milliseconds(static_cast<int>(nextFrameTime)));

                sws_scale(_swsContext, frame->data, frame->linesize, 0, _codecContext->height, _rgbFrameBuffer->data,
                    _rgbFrameBuffer->linesize);

                for (int s = 0; s < 3; s++) {
                    for (int i = 0; i < bufferSize / 3; i++) {
                        outBuffer[i * 3 + s] = _rgbFrameBuffer->data[0][s * bufferSize / 3 + i];
                    }
                }
                av_frame_unref(frame);
                av_packet_unref(&packet);

                return;
            }
        }
    }
}

void VideoDecoder::Replay() {
    av_seek_frame(_formatContext, _streamIndex, 0, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecContext);
    _startTime = std::chrono::steady_clock::now();
}
