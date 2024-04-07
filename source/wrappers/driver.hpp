#ifndef CONVENTION_NAMETAG_DRIVER_HPP
#define CONVENTION_NAMETAG_DRIVER_HPP

#include "hardware.hpp"

#include <algorithm> // std::max
#include <cstring>
#include <stdexcept>

/**
 * @brief SH1106
 * Notes to how it works
 *
 * Buffer to display mapping:
 * This seems to be arranged in a manner like
 *         ↓ bits
 * bytes → 1 2 3 .. 128
 *         2 2 2
 *         ..
 *         7 7 7
 *         8 8 8
 *         1 1 1
 *         ..
 *
 * Each vertical section of 8 bits is a page, there are 8 pages.
 */
namespace HardwareSpecs {
struct SH1106 {
    enum Dimensions : int {
        XMax = 132, // 132 in spec, but only 128 displayed?
        YMax = 64,
        XOffset = 2,
        YOffset = 0,

        Width = XMax - 2 * XOffset,
        Height = YMax,

        Size = Width * Height,
        BufferSize = Size / 8
    };

    struct Registry {
        enum Commands : int {
            /*
              "Specifies column address of display RAM. Divide the column
              address into 4 higher bits and 4 lower bits. Set each of them
              into successions. When the microprocessor repeats to access to
              the display RAM, the column address counter is incremented
              during each access until address 131 is accessed. The page
              address is not changed during this time." (spec p.19)

              Note LowColumn being 2 due to 128<->132 pixel discrepancy.
             */
            SelectColumnLow = 0x02,  // ( 0x0 -  0xF)
            SelectColumnHigh = 0x10, // (0x10 - 0x1F)

            /*
              Unknown, found in demo code.
             */
            // (0x00-0x02)
            SetPageAddressingMode = 0x20,

            /*
              "Specifies output voltage (VPP) of the internal charger pump."
              (spec p.19)

              0x30: 6.4V, 0x31: 7.4V, 0x32: 8.0V (Power On), 0x33: 9.0V
             */
            PumpVoltage = 0x30, // (0x30-0x33)

            /*
              "Specifies line address (refer to Figure. 8) to determine the
              initial display line or COM0. The RAM display data becomes the
              top line of OLED screen. It is followed by the higher number of
              lines in ascending order, corresponding to the duty cycle. When
              this command changes the line address, the smooth scrolling or
              page change takes place." (spec p.20)
             */
            SelectLine = 0x40, // (0x40 - 0x7F)

            /*
              "This command is to set contrast setting of the display. The
              chip has 256 contrast steps from 00 to FF. The segment output
              current increases as the contrast step value increases.
              Segment output current setting: ISEG = a/256 × IREF × scale
              factor, where: a is contrast step; IREF is reference current
              equals 12.5μA; Scale factor = 16." (spec p.20)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x00-0xFF)
             */
            SetContrastControl = 0x81,
            ValueContrastControlReset = 0x80,

            /*
              "Change the relationship between RAM column address and
              segment driver. The order of segment driver output pads can be
              reversed by software. This allows flexible IC layout during
              OLED module assembly. For details, refer to the column address
              section of Figure. 8. When display data is written or read, the
              column address is incremented by 1 as shown in Figure. 1."
              (spec p.21)
             */
            SegmentRemapNormal = 0xA0,
            SegmentRemapReverse = 0xA1,

            /*
              "Forcibly turns the entire display on regardless of the
              contents of the display data RAM. At this time, the contents
              of the display data RAM are held." (spec p.21)
             */
            DisableForceDisplayOn = 0xA4,
            EnableForceDisplayOn = 0xA5,

            /*
              "Reverses the display ON/OFF status without rewriting the
              contents of the display data RAM." (spec p.21)
             */
            DisableInverseDisplay = 0xA6,
            EnableInverseDisplay = 0xA7,

            /*
              "This command switches default 64 multiplex modes to any
              multiplex ratio from 1 to 64. The output pads COM0-COM63 will
              be switched to corresponding common signal." (spec p.22)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x00-0x3F)
             */
            SetMultiplexRatio = 0xA8,
            ValueMultiplexRatioDefault = 0x3F,

            /*
              "This command is to control the DC-DC voltage converter. The
              converter will be turned on by issuing this command then
              display ON command. The panel display must be off while issuing
              this command." (spec p.22)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x8A-0x8B)

              (Why did they make a setter for a 0-1 range?)
             */
            SetDCDC = 0xAD,

            /*
              "Alternatively turns the display on and off." (spec p. 23)
             */
            PanelOff = 0xAE,
            PanelOn = 0xAF,

            /*
              "Specifies page address to load display RAM data to page
              address register. Any RAM data bit can be accessed when its
              page address and column address are specified. The display
              remains unchanged even when the page address is changed."
              (spec p.23)
             */
            Page = 0xB0, // 0xB0 - 0xB7

            /*
              "This command sets the scan direction of the common output
              allowing layout flexibility in OLED module design. In addition,
              the display will have immediate effect once this command is
              issued. That is, if this command is sent during normal display,
              the graphic display will be vertically flipped." (spec p.24)

              "When D = “L”, Scan from COM0 to COM [N -1]. (POR)
              When D = “H”, Scan from COM [N -1] to COM0." (spec p.24)
              Where D = 4th significant bit
            */
            ComRowScanDirection = 0xC0, // (0xC0-0xC8), (0xC9-0xCF for reversed?)

            // unused: 0xC1-0xCF

            /*
              "The next command specifies the mapping of display start line
              to one of COM0-63 (it is assumed that COM0 is the display start
              line, that equals to 0). For example, to move the COM16 towards
              the COM0 direction for 16 lines, the 6-bit data in the second
              byte should be given by 010000. To move in the opposite
              direction by 16 lines, the 6-bit data should be given by
              (64-16), so the second byte should be 100000." (spec p.24)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x00-0x3F)
             */
            SetDisplayOffset = 0xD3,
            ValueDisplayOffsetDefault = 0x0,

            /*
              "This command is used to set the frequency of the internal
              display clocks (DCLKs). It is defined as the divide ratio
              (Value from 1 to 16) used to divide the oscillator frequency.
              POR is 1. Frame frequency is determined by divide ratio, number
              of display clocks per row, MUX ratio and oscillator frequency."
              (spec p.25)

              This is a double-byte ("set") command. The next input will be
              used as value for the register.
              (range: 0x0-0xF divide ratio + 0x10-0xF0 oscillator frequency)
             */
            SetDisplayClockFreq = 0xD5,
            ValueDisplayClockFreqDefault = 0b01010000,

            /*
              "This command is used to set the duration of the pre-charge
              period. The interval is counted in number of DCLK. POR is 2
              DCLKs." (spec p.26)

              This is a double-byte ("set") command. The next input will be
              used as value for the register.
              (range: 0x0-0xF precharge period +
                      0x10-0xF0 discharge period)
             */
            SetChargePeriod = 0xD9,
            ValueChargePeriodDefault = 0x22,

            /*
              "This command is to set the common signals pad configuration
              (sequential or alternative) to match the OLED panel hardware
              layout" (spec p.26)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x02, 0x12)
             */
            SetComPinsHWConf = 0xDA,
            ValueComPinsHWConfDefault = 0x12,

            /*
              "This command is to set the common pad output voltage level
              at deselect stage." (spec p.27)

              This is a double-byte ("set") command. The next input will be
              used as value for the register. (range: 0x00-0xFF)

              Resulting VCOM = β × VREF = (0.430 + <value> × 0.006415) × VREF
             */
            SetVCOMH = 0xDB,
            ValueVCOMHDefault = 0x35,

            /*
              "A pair of Read-Modify-Write and End commands must always be
              used. Once read-modify-write is issued, column address is not
              incremental by read display data command but incremental by
              write display data command only. It continues until End command
              is issued. When the End is issued, column address returns to
              the address when read-modify-write is issued. This can reduce
              the microprocessor load when data of a specific display area
              is repeatedly changed during cursor blinking or others."
              (spec p.28)
             */
            ReadModifyWriteOn = 0xE0,
            ReadModifyWriteOff = 0xEE,

            /*
              "Non-Operation Command." (spec p.29)
             */
            Nop = 0xE3
        };
    };
};

struct SSD1305 {
    enum Dimensions : int {
        XMax = 128,
        YMax = 32,
        XOffset = 0,
        YOffset = 0,

        Width = XMax,
        Height = YMax,

        Size = Width * Height,
        BufferSize = Size / 8
    };

    struct Registry {
        /**
         * Commands as per SSD1305 spec Rev 1.9
         */
        enum Commands : int {
            // p. 40
            SelectColumnLow = 0x00,  // [0x0-0xF]
            SelectColumnHigh = 0x10, // [0x0-0xF]

            SetMemoryAddressingMode = 0x20,

            // p. 43
            SetDisplayStartLine = 0x40,
            SetContrastControl = 0x81,

            // p. 44
            SetSegmentRemap = 0xA0, // [0x0-0x1]

            DisableInverseDisplay = 0xA6,
            EnableInverseDisplay = 0xA7,

            SetMultiplexRatio = 0xA8,

            // p. 45
            PanelDim = 0xAC,
            PanelOff = 0xAE,
            PanelOn = 0xAF,

            SetComOutputScanDir = 0xC0 + 0x8,

            SetDisplayOffset = 0xD3,

            // also known as clock divide ratio
            SetDisplayOscillatorFrequency = 0xD5,

            SetAreaColorMode = 0xD8,

            SetPrechargePeriod = 0xD9,
            SetCOMPinsHardwareConfiguration = 0xDA,
            SetVComH = 0xDB
        };
    }; // namespace Registry
};     // namespace SSD1305

/**
 * @brief SSD1322
 *
 * Pixels are 4 bits, a byte contains 2 adjacent pixels-
 *
 */
struct SSD1322 {
    enum Dimensions : int {
        XMax = 256,
        YMax = 64,
        XOffset = 0,
        YOffset = 0,

        Width = XMax,
        Height = YMax,

        Size = Width * Height,
        BufferSize = Size / 2
    };

    struct Registry {
        enum Commands : int {
            SetColumnAddress = 0x15,
            SetRowAddress = 0x75,

            WriteRam = 0x5C,

            SetRemap = 0xA0,
            SetStartLine = 0xA1,
            SetDisplayOffset = 0xA2,
            DisableForceDisplayOn = 0xA4,
            EnableForceDisplayOn = 0xA5,
            DisableInverseDisplay = 0xA6,
            EnableInverseDisplay = 0xA7,
            ExitPartialDispaly = 0xA9,
            FunctionSelect = 0xAB,
            PanelOff = 0xAE,
            PanelOn = 0xAF,
            SetPhaseLength = 0xB1,
            SetClockDivider = 0xB3,
            DisplayEnhance = 0xB4,
            SetGPIO = 0xB5,
            SetSecondPrechargePeriod = 0xB6,
            SelectDefaultGrayscale = 0xB9,
            SetPrechargeVoltage = 0xBB,
            SetVComH = 0xBE,
            SetContrastCurrent = 0xC1,
            MasterCurrentControl = 0xC7,
            SetMultiplexRatio = 0xCA,
            DisplayEnhanceB = 0xD1,
            SetCommandLock = 0xFD,

        };
    };
}; // namespace SSD1322
} // namespace HardwareSpecs

namespace Wrappers {
/**
 * @brief Abstraction for basic program-display-controller interaction
 */
template <class DeviceType> class Driver {
  public:
    using ColorT = uint16_t;

    struct Pins {
        enum I2CCommands : int { IICCMD = 0x00, IICRAM = 0x40 };
    };

    Driver() {
        if (not Hardware::Init()) {
            throw std::runtime_error("failed to initialize hardware");
        }

        Reset();
    }
    virtual ~Driver() {
        SetPanelPower(false);
        Hardware::Exit();
    }

    void SetCursor(uint8_t x, uint8_t y) {
        if ((x > _width) || (y > _height)) {
            return;
        }

        // set page address
        WriteRegistry(DeviceType::Registry::Page + (y / 8));
        // set low column address
        WriteRegistry(DeviceType::Registry::SelectColumnLow + (x & 0x0f));
        // set high column address
        WriteRegistry(DeviceType::Registry::SelectColumnHigh + (x >> 4));

        /*
         * Cursor position is now set... but what does it do?
         */
    }

    void SetColor(uint8_t x, uint8_t y, ColorT color) {
        // TODO: handle color == black

        if (x >= _width || y >= _height) {
            return;
        }

        _buffer[x + (y / 8) * _width] |= static_cast<uint8_t>(color != 0) << (y % 8);
    }

    void Clear(ColorT color = 0) { std::memset(_buffer, color, sizeof(_buffer)); }

    // used in constructor, cannot be pure virtual
    virtual void Display(){};

    void SetPanelPower(bool on = true) {
        if (on) {
            WriteRegistry(DeviceType::Registry::PanelOn);
        } else {
            WriteRegistry(DeviceType::Registry::PanelOff);
        }
    }

    virtual void CopyFramebuffer(const uint8_t *glBuffer) = 0;

    [[nodiscard]] int GetWidth() const { return _width; }
    [[nodiscard]] int GetHeight() const { return _height; }

  protected:
    void WriteRegistry(uint8_t reg) {
        if constexpr (Hardware::UseSPI) {
            Hardware::DC0();
            Hardware::SPIWriteByte(reg);
        } else {
            Hardware::I2CWriteByte(reg, Pins::IICCMD);
        }
    }

    void WriteDataByte(uint8_t data) {
        if constexpr (Hardware::UseSPI) {
            Hardware::DC1();
            Hardware::SPIWriteByte(data);
        } else {
            Hardware::I2CWriteByte(data, Pins::IICRAM);
        }
    }

    void WriteData(uint8_t *buffer, uint32_t length) {
        if constexpr (Hardware::UseSPI) {
            Hardware::DC1();
            Hardware::SPIWriteBytes(reinterpret_cast<char *>(buffer), length);
        } else {
            for (uint32_t i{0}; i < length; buffer++, i++) {
                Hardware::I2CWriteByte(*buffer, Pins::IICRAM);
            }
        }
    }

    void Reset() {
        Hardware::RST1();
        Hardware::DelayMS(100);
        Hardware::RST0();
        Hardware::DelayMS(400);
        Hardware::RST1();
        Hardware::DelayMS(100);
    }

    // 1-4 bpp buffer
    uint8_t _buffer[DeviceType::BufferSize]{};
    int _width{DeviceType::Width};
    int _height{DeviceType::Height};
};

class [[maybe_unused]] SH1106 : public Driver<HardwareSpecs::SH1106> {
  public:
    struct Pins : public Driver<HardwareSpecs::SH1106> {
        enum PinIndices : int {
            KeyUpPin = 6,
            KeyDownPin = 19,
            KeyLeftPin = 5,
            KeyRightPin = 26,
            KeyPressPin = 13,
            Key1Pin = 21,
            Key2Pin = 20,
            Key3Pin = 16
        };
    };

    SH1106();
    void Display() override;
    void CopyFramebuffer(const uint8_t *glBuffer) override;

    uint8_t GetKeyUp();
    uint8_t GetKeyDown();
    uint8_t GetKeyLeft();
    uint8_t GetKeyRight();
    uint8_t GetKeyPress();
    uint8_t GetKey1();
    uint8_t GetKey2();
    uint8_t GetKey3();

  private:
    void InitRegistry();
};

class [[maybe_unused]] SSD1322 : public Driver<HardwareSpecs::SSD1322> {
  public:
    SSD1322();

    void Display() override;
    void CopyFramebuffer(const uint8_t *glBuffer) override;

  private:
    void InitRegistry();
};

class [[maybe_unused]] SSD1305 : public Driver<HardwareSpecs::SSD1305> {
  public:
    SSD1305();

    void Display() override;
    void CopyFramebuffer(const uint8_t *glBuffer) override;

  private:
    void InitRegistry();
};
} // namespace Wrappers

#endif