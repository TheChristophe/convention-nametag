#include "hardware.hpp"

#include <cstdio>

namespace Hardware {
bool Init() {
#ifndef DEV_MODE
    if (bcm2835_init() == 0) {
        return false;
    }

    // setup pins
    EnablePin(RST);
    EnablePin(DC);
    EnablePin(CS);

    if constexpr (UseSPI) {
        printf("Using SPI\n");
        // init
        bcm2835_spi_begin();
        // transmission order
        bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
        /*
        BCM2835_SPI_MODE0 CPOL = 0, CPHA = 0, Clock idle low, data is clocked in
        on rising edge, output data (change) on falling edge BCM2835_SPI_MODE1
        CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge,
        output data (change) on rising edge BCM2835_SPI_MODE2 CPOL = 1, CPHA =
        0, Clock idle high, data is clocked in on falling edge, output data
        (change) on rising edge BCM2835_SPI_MODE3 CPOL = 1, CPHA = 1, Clock idle
        high, data is clocked in on rising, edge output data (change) on falling
        edge
        */
        bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
        // 32 is lowest compatible library value for SSD1322
        // bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
        // 18 is lowest experimentally determined working value for ssd1322
        // but 20 is more stable
        bcm2835_spi_setClockDivider(20);
        // select 0
        bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
        // enable 0
        bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
    } else if constexpr (UseIIC) {
        DC0();
        CS0();
        printf("Using IIC\n");
        bcm2835_i2c_begin();
        bcm2835_i2c_setSlaveAddress(0x3c); // i2c address
        // bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_148);
        bcm2835_i2c_set_baudrate(100000); // 100k baudrate
    }
#endif
    return true;
}

} // namespace Hardware