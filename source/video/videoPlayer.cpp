#include "videoPlayer.hpp"
#include "videoDecoder.hpp"

VideoPlayer::VideoPlayer(int width, int height) : _width{width}, _height{height} {}

void VideoPlayer::FetchFrame(uint8_t *buffer, int bufferSize) {
    // TODO: "faster" alternatives to locking every frame?
    auto lock = std::lock_guard<std::mutex>(_decoderAccess);
    _activeDecoder->DecodeFrame(buffer, bufferSize);
}

bool VideoPlayer::PlayFile(const std::filesystem::path &file) {
    // clean up previous player first
    // TODO: any ffmpeg components we can reuse?
    if (std::filesystem::exists(file) && std::filesystem::is_regular_file(file)) {
        auto lock = std::lock_guard<std::mutex>(_decoderAccess);
        _activeDecoder.reset();
        _activeDecoder = std::make_unique<VideoDecoder>(file, _width, _height);
        return true;
    }
    return false;
}
