#ifndef CONVENTION_NAMETAG_IDLEDECODER_HPP
#define CONVENTION_NAMETAG_IDLEDECODER_HPP

#include "decoder.hpp"

class IdleDecoder : public Decoder {
  public:
    ~IdleDecoder() override = default;

    void DecodeFrame(uint8_t *buffer, int bufferSize) override;
};

#endif // CONVENTION_NAMETAG_IDLEDECODER_HPP
