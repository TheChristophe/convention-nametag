#ifndef CONVENTION_NAMETAG_DECODER_HPP
#define CONVENTION_NAMETAG_DECODER_HPP

#include <cstddef>
#include <cstdint>

class Decoder {
  public:
    virtual ~Decoder() = default;

    virtual void DecodeFrame(uint8_t *buffer, int bufferSize) = 0;

  private:
};

#endif
