#include "driver.hpp"

namespace Wrappers {
SH1106::SH1106() : Driver<HardwareSpecs::SH1106>::Driver() {
    Hardware::EnablePin(Pins::KeyUpPin);
    Hardware::EnablePin(Pins::KeyDownPin);
    Hardware::EnablePin(Pins::KeyLeftPin);
    Hardware::EnablePin(Pins::KeyRightPin);
    Hardware::EnablePin(Pins::KeyPressPin);
    Hardware::EnablePin(Pins::Key1Pin);
    Hardware::EnablePin(Pins::Key2Pin);
    Hardware::EnablePin(Pins::Key3Pin);

    // cannot put next lines in common constructor calling virtual from
    // constructor breaks
    InitRegistry();

    Hardware::DelayMS(200);

    Clear();
    SetPanelPower(true);
}

void SH1106::Display() {
    uint8_t *buffer{_buffer};

    for (uint8_t page{0}; page < 8; page++, buffer += _width) {
        // set page address
        WriteRegistry(HardwareSpecs::SH1106::Registry::Page + page);

        // reset column address
        WriteRegistry(HardwareSpecs::SH1106::Registry::SelectColumnLow);
        WriteRegistry(HardwareSpecs::SH1106::Registry::SelectColumnHigh);

        // write display buffer
        WriteData(buffer, _width);
    }
}

void SH1106::CopyFramebuffer(const uint8_t *glBuffer) {
    // gl buffer is formatted in RGB, so multiply index by 3 to use R channels

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

uint8_t SH1106::GetKeyUp() { return Hardware::ReadPin(Pins::KeyUpPin); }

uint8_t SH1106::GetKeyDown() { return Hardware::ReadPin(Pins::KeyDownPin); }

uint8_t SH1106::GetKeyLeft() { return Hardware::ReadPin(Pins::KeyLeftPin); }

uint8_t SH1106::GetKeyRight() { return Hardware::ReadPin(Pins::KeyRightPin); }

uint8_t SH1106::GetKeyPress() { return Hardware::ReadPin(Pins::KeyPressPin); }

uint8_t SH1106::GetKey1() { return Hardware::ReadPin(Pins::Key1Pin); }

uint8_t SH1106::GetKey2() { return Hardware::ReadPin(Pins::Key2Pin); }

uint8_t SH1106::GetKey3() { return Hardware::ReadPin(Pins::Key3Pin); }

void SH1106::InitRegistry() {
    namespace HW = HardwareSpecs;

    WriteRegistry(HW::SH1106::Registry::PanelOff);

    WriteRegistry(HW::SH1106::Registry::SelectColumnLow + 0x0);
    WriteRegistry(HW::SH1106::Registry::SelectColumnHigh + 0x0);

    WriteRegistry(HW::SH1106::Registry::SelectLine + 0x0);

    WriteRegistry(HW::SH1106::Registry::SetContrastControl);
    WriteRegistry(HW::SH1106::Registry::ValueContrastControlReset);

    WriteRegistry(HW::SH1106::Registry::SegmentRemapNormal);
    WriteRegistry(HW::SH1106::Registry::ComRowScanDirection + 0x0);

    WriteRegistry(HW::SH1106::Registry::SetMultiplexRatio);
    WriteRegistry(HW::SH1106::Registry::ValueMultiplexRatioDefault);

    WriteRegistry(HW::SH1106::Registry::SetDisplayOffset);
    WriteRegistry(HW::SH1106::Registry::ValueDisplayOffsetDefault);

    WriteRegistry(HW::SH1106::Registry::SetDisplayClockFreq);
    WriteRegistry(HW::SH1106::Registry::ValueDisplayClockFreqDefault);

    WriteRegistry(HW::SH1106::Registry::SetChargePeriod);
    WriteRegistry(0xF1);

    WriteRegistry(HW::SH1106::Registry::SetComPinsHWConf);
    WriteRegistry(HW::SH1106::Registry::ValueComPinsHWConfDefault);

    WriteRegistry(HW::SH1106::Registry::SetVCOMH);
    WriteRegistry(0x40); // Set VCOM Deselect Level

    WriteRegistry(HW::SH1106::Registry::SetPageAddressingMode);
    WriteRegistry(0x02);

    WriteRegistry(HW::SH1106::Registry::DisableForceDisplayOn);
    WriteRegistry(HW::SH1106::Registry::DisableInverseDisplay);
}
} // namespace Wrappers