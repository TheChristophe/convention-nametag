#include "idleDecoder.hpp"

#include <chrono>
#include <thread>

void IdleDecoder::DecodeFrame(uint8_t *buffer, int bufferSize)
{
    // noop, but don't go too fast
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
}
