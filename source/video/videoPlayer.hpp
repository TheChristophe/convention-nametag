#ifndef CONVENTION_NAMETAG_VIDEOPLAYER_HPP
#define CONVENTION_NAMETAG_VIDEOPLAYER_HPP

#include "decoder.hpp"
#include "idleDecoder.hpp"

#include <filesystem>
#include <memory>
#include <mutex>

class VideoPlayer {
  public:
    VideoPlayer(int width, int height);
    ~VideoPlayer() = default;

    void FetchFrame(uint8_t *buffer, int bufferSize);
    bool PlayFile(const std::filesystem::path &file);

  private:
    std::unique_ptr<Decoder> _activeDecoder{std::make_unique<IdleDecoder>()};

    int _width;
    int _height;

    std::mutex _decoderAccess;
};

#endif // CONVENTION_NAMETAG_VIDEOPLAYER_HPP
