#include "driver.hpp"

namespace Wrappers {
SSD1322::SSD1322() {
    // cannot put next lines in common constructor calling virtual from
    // constructor breaks
    InitRegistry();

    Hardware::DelayMS(200);

    Clear();
    SetPanelPower(true);
}

void SSD1322::Display() {
    WriteRegistry(HardwareSpecs::SSD1322::Registry::SetColumnAddress);
    const auto ColOffset{0x1C};
    WriteDataByte(static_cast<uint8_t>(ColOffset + 0x00));
    WriteDataByte(static_cast<uint8_t>(ColOffset + 0x3F));

    WriteRegistry(HardwareSpecs::SSD1322::Registry::SetRowAddress);
    WriteDataByte(0x00);
    WriteDataByte(0x3F);

    WriteRegistry(HardwareSpecs::SSD1322::Registry::WriteRam);

    WriteData(_buffer, HardwareSpecs::SSD1322::BufferSize);

    Hardware::DelayMS(0);
}

void SSD1322::CopyFramebuffer(const uint8_t *glBuffer) {
    // gl buffer is formatted in RGB, so multiply index by 3 to use R channels
    for (unsigned y{0}; y < 64; y++) {
        for (unsigned x{0}; x < 256; x += 2) {
            _buffer[(y * _width + x) / 2] = 0;

            // This is what I used for openGL
            // this doesn't work for my ffmpeg output
            // TODO: figure out why this even worked in the first place (was
            // opengl output flipped horizontally?)
            //_buffer[(y * _state.width + x) / 2] |= glBuffer[((63 - y) *
            //_state.width + x) * 3] & 0xF0; _buffer[(y * _state.width + x) / 2]
            //|= glBuffer[((63 - y) * _state.width + x + 1) * 3] >> 4;

            // left pixel is 4 high bits
            _buffer[(y * _width + x) / 2] |= glBuffer[(y * _width + x) * 3] & 0xF0;
            // right pixel is 4 low bits
            _buffer[(y * _width + x) / 2] |= glBuffer[(y * _width + x + 1) * 3] >> 4;
        }
    }
}

void SSD1322::InitRegistry() {
    namespace HW = HardwareSpecs;

    WriteRegistry(HW::SSD1322::Registry::SetCommandLock);
    WriteDataByte(0x12); // Unlock OLED driver IC

    WriteRegistry(HW::SSD1322::Registry::PanelOff);

    WriteRegistry(HW::SSD1322::Registry::SetClockDivider);
    WriteDataByte(0x91);

    WriteRegistry(HW::SSD1322::Registry::SetMultiplexRatio);
    WriteDataByte(0x3F); // duty = 1/64

    WriteRegistry(HW::SSD1322::Registry::SetDisplayOffset);
    WriteDataByte(0x00);

    WriteRegistry(HW::SSD1322::Registry::SetStartLine);
    WriteDataByte(0x00);

    WriteRegistry(HW::SSD1322::Registry::SetRemap);
    WriteDataByte(0x14); // Horizontal address increment,Disable Column Address
                         // Re-map,Enable Nibble Re-map,Scan from COM[N-1] to
                         // COM0,Disable COM Split Odd Even
    WriteDataByte(0x11); // Enable Dual COM mode

    WriteRegistry(HW::SSD1322::Registry::SetGPIO);
    WriteDataByte(0x00); // Disable GPIO Pins Input

    WriteRegistry(HW::SSD1322::Registry::FunctionSelect);
    WriteDataByte(0x01); // selection external vdd

    WriteRegistry(HW::SSD1322::Registry::DisplayEnhance);
    WriteDataByte(0xA0); // enables the external VSL
    WriteDataByte(0xFD); // 0xfFD,Enhanced low GS display quality;default is 0xb5(normal),

    WriteRegistry(HW::SSD1322::Registry::SetContrastCurrent);
    WriteDataByte(0xFF); // 0xFF - default is 0x7f

    WriteRegistry(HW::SSD1322::Registry::MasterCurrentControl);
    WriteDataByte(0x0F); // default is 0x0F

    WriteRegistry(HW::SSD1322::Registry::SelectDefaultGrayscale);

    WriteRegistry(HW::SSD1322::Registry::SetPhaseLength);
    WriteDataByte(0xE2); // default is 0x74

    WriteRegistry(HW::SSD1322::Registry::DisplayEnhanceB);
    WriteDataByte(0x82); // Reserved;default is 0xa2(normal)
    WriteDataByte(0x20); //

    WriteRegistry(HW::SSD1322::Registry::SetPrechargeVoltage);
    WriteDataByte(0x1F); // 0.6xVcc

    WriteRegistry(HW::SSD1322::Registry::SetSecondPrechargePeriod);
    WriteDataByte(0x08); // default

    WriteRegistry(HW::SSD1322::Registry::SetVComH);
    WriteDataByte(0x04); // 0.86xVcc;default is 0x04

    WriteRegistry(HW::SSD1322::Registry::DisableInverseDisplay);

    WriteRegistry(HW::SSD1322::Registry::ExitPartialDispaly);
}
} // namespace Wrappers