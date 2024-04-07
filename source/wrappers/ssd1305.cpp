#include "driver.hpp"

namespace Wrappers {
SSD1305::SSD1305() {
    // cannot put next lines in common constructor calling virtual from
    // constructor breaks
    InitRegistry();

    Hardware::DelayMS(200);

    Clear();
    SetPanelPower(true);
}

void SSD1305::Display() {
    uint8_t *buffer{_buffer};

    for (uint8_t page{0}; page < 8; page++, buffer += _width) {
        // set page address
        // TODO: why no HardwareSpaces::SSD1305::Registry::Page?
        WriteRegistry(HardwareSpecs::SH1106::Registry::Page + page);

        // reset column address
        WriteRegistry(0x04);
        WriteRegistry(HardwareSpecs::SSD1305::Registry::SelectColumnHigh);

        // write display buffer
        WriteData(buffer, _width);
    }
}

void SSD1305::CopyFramebuffer(const uint8_t *glBuffer) {
    for (int page{0}; page < 8; page++) {
        for (int x{0}; x < _width; x++) {
            const auto bufferIndex{x + page * _width};
            _buffer[bufferIndex] = 0;
        }

        for (int y{0}; y < 8; y++) {
            for (int x{0}; x < _width; x++) {
                // set pixel on if at least 50% bright
                const uint8_t bitValue{glBuffer[(page * _width * 8 + y * _width + x) * 3] > 0x7F};
                _buffer[page * _width + x] |= bitValue << y;
            }
        }
    }
}

void SSD1305::InitRegistry() {
    namespace HW = HardwareSpecs;

    WriteRegistry(HW::SSD1305::Registry::PanelOff);
    WriteRegistry(HW::SSD1305::Registry::SelectColumnLow + 0x04);
    WriteRegistry(HW::SSD1305::Registry::SelectColumnHigh + 0x00);
    WriteRegistry(HW::SSD1305::Registry::SetDisplayStartLine);
    WriteRegistry(HW::SSD1305::Registry::SetContrastControl);
    WriteRegistry(0x80); // ?
    WriteRegistry(HW::SSD1305::Registry::SetSegmentRemap + 0x1);
    WriteRegistry(HW::SSD1305::Registry::DisableInverseDisplay);
    WriteRegistry(HW::SSD1305::Registry::SetMultiplexRatio);
    WriteRegistry(0x1F);
    WriteRegistry(HW::SSD1305::Registry::SetComOutputScanDir);
    WriteRegistry(HW::SSD1305::Registry::SetDisplayOffset);
    WriteRegistry(0x00);
    WriteRegistry(HW::SSD1305::Registry::SetDisplayOscillatorFrequency);
    WriteRegistry(0xF0);
    WriteRegistry(HW::SSD1305::Registry::SetAreaColorMode);
    WriteRegistry(0x05);
    WriteRegistry(HW::SSD1305::Registry::SetPrechargePeriod);
    WriteRegistry(0xC2); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    WriteRegistry(HW::SSD1305::Registry::SetCOMPinsHardwareConfiguration);
    WriteRegistry(0x12);
    WriteRegistry(HW::SSD1305::Registry::SetVComH);
    WriteRegistry(0x08);
}
} // namespace Wrappers