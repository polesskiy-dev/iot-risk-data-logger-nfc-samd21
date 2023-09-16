# IoT Risk Data Logger

![](https://github.com/polesskiy-dev/iot-risk-data-logger-nfc-samd21-hardware/raw/main/docs/assets/logger-assembly_v7_2023-Aug-21_06-55-26PM-000_CustomizedView26145487162.png)

## Introduction
Fully open-source and open-hardware **data logger** tailored for logistics applications i.e. for cold chain monitoring systems.

## Hardware

[Schematics, PCB, enclosure, BOM, assembly documentation](https://github.com/polesskiy-dev/iot-risk-data-logger-nfc-samd21-hardware)

### Features

#### Key risk metrics:
- Temperature/Humidity [Sensirion SHT3x](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf)
- Ambient Light Sensor [TI OPT3001](https://www.ti.com/lit/ds/symlink/opt3001.pdf?ts=1694786594042&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FOPT3001)
- Shock Sensor - Accelerometer [NXP MMA8452Q](https://www.nxp.com/docs/en/data-sheet/MMA8452Q.pdf)

#### Communication:
- USB-C:
    - MSD: Device works ad a flash drive to direct log read on PC
    - CDC: Console for debug and firmware bootloader
- NFC for control through mobile app [ST ST25DV04K](https://www.st.com/resource/en/datasheet/st25dv04k.pdf)

#### Security:
- Hardware crypto [Microchip ATECC608A](https://www.microchip.com/wwwproducts/en/ATECC608A) for:
    - Firmware encryption for bootloader
    - Log data encryption
    - Authentication (X509 certificates for IoT platforms)

#### Power:
- Powered by 2xAAA batteries or USB-C (reverse polarity protection, OR source selection)
- Low Power Consumption: Ultra-low power architecture, ~ one year of data acquisition

#### Additional Features:
- 4MB SPI NOR Flash [AT25DF321](https://www.mouser.com/catalog/specsheets/atmel_doc3669.pdf?_gl=1*5ifz1l*_ga*NTg5MTc1OTE2LjE2ODkwMTAyMDY.*_ga_15W4STQT4T*MTY5NDg2NDM4MS4yMS4wLjE2OTQ4NjQzODIuMC4wLjA.*_ga_1KQLCYKRX3*MTY5NDg2NDM4MS4yMS4wLjE2OTQ4NjQzODIuNTkuMC4w)
- RTC Clock: accurate 32.768Hz quartz oscillator
- LED: Simple indication for device states

#### Main MCU
- [Microchip ATSAMD21E18](https://www.microchip.com/en-us/product/atsamd21e18) - 32-bit ARM Cortex-M0+ MCU, 48MHz, 256KB of flash and 32KB of SRAM. Satisfies AEC-Q100, recommended for Automotive.

## Firmware

The firmware is written in C with a bare-metal approach and exclusively uses static memory allocation. It is developed on top of the [Harmony v3](https://www.microchip.com/en-us/tools-resources/configure/mplab-harmony) Embedded Software framework. Developed with [MPLAB X](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide), [XC32](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers) compiler.

### Main architecture

The source code adheres to an asynchronous, non-blocking programming style designed to achieve ultra-low power consumption.

The core mechanism revolves around handling asynchronous events from a queue and entering a low-power sleep state when the queue is empty. New events are enqueued either through interrupts (e.g., cron-scheduled sensor reads) or through the main system loop.

The architecture incorporates the following key concepts:
- [Event Loop](https://en.wikipedia.org/wiki/Event_loop)
- [Actor Model](https://en.wikipedia.org/wiki/Actor_model) - aka [Active Object](https://en.wikipedia.org/wiki/Active_object)
- [Finite-state machine](https://en.wikipedia.org/wiki/Finite-state_machine) - [Mealy](https://en.wikipedia.org/wiki/Mealy_machine) + [Moore](https://en.wikipedia.org/wiki/Moore_machine) implementation

To streamline development, an Active Object + FSM library has been implemented and is maintained in a separate [active-object-fsm](https://github.com/polesskiy-dev/active-object-fsm) repository.

## Installation
    
    $ git clone https://github.com/polesskiy-dev/iot-risk-data-logger-nfc-samd21 --recurse-submodules

Update submodules to the latest version (optional):

    $ git submodule init && git submodule update --remote # install submodules

## Some dev process photos:

![DB327772-9ED3-43A9-9017-51C4E369E922_1_105_c.jpeg](docs%2Fdev-process%2FDB327772-9ED3-43A9-9017-51C4E369E922_1_105_c.jpeg)
![A57A5346-DAB0-40F0-AA93-4D77E1BF0848_1_105_c.jpeg](docs%2Fdev-process%2FA57A5346-DAB0-40F0-AA93-4D77E1BF0848_1_105_c.jpeg)

