#include "helper.hpp"

extern "C" {
#include <libavformat/avformat.h>
}

#include <cstring>
#include <format>
#include <iostream>

std::optional<double> getVideoDuration(const std::filesystem::path &fileName) {
    AVFormatContext *context = avformat_alloc_context();
    if (const auto r = avformat_open_input(&context, fileName.c_str(), nullptr, nullptr); r != 0) {
        std::cerr << std::format(
                         "Error while trying to read duration of {}: {}", fileName.c_str(), std::strerror(AVERROR(r)))
                  << std::endl;
        return std::nullopt;
    }

    double duration{static_cast<double>(context->duration) / AV_TIME_BASE};

    // TODO: why is context-duration fucked
    // if (duration < 0) {
    for (int i = 0; i < static_cast<int>(context->nb_streams); i++) {
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            auto stream = context->streams[i];
            duration = static_cast<double>(stream->duration) *
                       (static_cast<double>(stream->time_base.num) / stream->time_base.den);
            break;
        }
    }
    //}

    avformat_close_input(&context);
    avformat_free_context(context);

    return duration;
}