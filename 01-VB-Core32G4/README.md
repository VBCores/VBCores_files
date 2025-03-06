 VBCores VB32G422

## Overview
The VBCores VB32G4 combines the processing power and rich peripherals of an STM32G474RE microcontroller with an 60V input power supply and FDCAN transceiver, all in a tiny (30.48mm x 38.1mm) form factor suitable for integration into products. It provides an affordable, convenient and flexible way to create electronic hardware in bot
h development and production stages. Any embedded software developer can use this device with ease thanks to compatibility with Arduino IDE (STM32duino), STM32Cube and all other products compatible with STM32G4 MCUs.

[![VB32G4 Schematic](https://raw.githubusercontent.com/VBCores/VBCores_files/refs/heads/main/01-VB-Core32G4/Vbcore-G474RE-print_v1_3.png)](https://raw.githubusercontent.com/VBCores/VBCores_files/refs/heads/main/01-VB-Core32G4/Vbcore-G474RE-print_v1_3.png)

### What's On Board
 1. **STM32G474RE:** the high performance STM32 MCU which features:
    -   **Core:** Cortex-M4 32-bit RISC
    -   **Operating Frequency:** 170MHz, 213 DMIPS
    -   **Package:** LQFP64
    -   **Memories:** 512kB Flash, 96kB SRAM, 32kB CCM SRAM
    -   **MCU communication Interfaces:**
        -   4 x SPI, 3 x USART, 2 x UART, 2 x I2S, 4 x I2C, 3 x FDCAN
        -   1 x USB 2.0 full-speed interface with LPM and BCD support
    -   **AD & DA converters:** 5 x AD (12-bit); 1 x DA (12-bit)
 2. **TPS54160:** 5V DC-DC converter
 3. **AP2112K-3.3:** 3.3V voltage regulator
 4. **MCP2542FD:** FDCAN transceiver, PB8-PB9
 5. **SWD+UART:** JST-GH 1.25 (6 pin)
 6. **User LEDs:** PA5, PD2
 7. **User Button:** PC13
 8. **Power indicator LED**
 9. **MCU Reset button**

### Features
- **Power Input**: 6-60V, reverse polarity protection, TVS protection
- **Power Output**: 5V 1.5A, 3.3V 0.6A
- **FDCAN**: up to 8MBit/s data bitrate, TVS protection
- **HSE**: 8MHz crystall
- **LSE**: 32.768KHz crystall

###  Dimensions
- 30.48mm x 38.1mm


### Schematic

[![VB32G4 Schematic](https://raw.githubusercontent.com/VBCores/VBCores_files/refs/heads/main/01-VB-Core32G4/VBCore32G4_schematic.png)](https://raw.githubusercontent.com/VBCores/VBCores_files/refs/heads/main/01-VB-Core32G4/VBCore32G4_schematic.png)


## SWD Interface

JST GH1.25, 6pin

| Pin      | Is           | 
| -------- | -------------|
| 1        | GND          |
| 2        | 5V           |
| 3        | SWCLK        |
| 4        | SWDIO        |
| 5        | TX USART2    |
| 6        | RX USART2    |










