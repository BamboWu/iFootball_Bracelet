This directory contains **HAL** (Hardware Abstract Layer) files.  
HAL work similarly as firmware library in STM32. It is some MACRO defines and functions concerning peripherals. nRF51822 has peripherials as following:  
 * ADC
 * clock
 * ECB (AES Electronic Codebook mode encryption)
 * *EGU (Event Generator Unit)*
 * gpio
 * gpiote (task & event)
 * LPComp (Low Power Comparetor)
 * NVMC (Non-Volatile Memory Controller)
 * PPI (Programmable Peripheral Interconnect)
 * QDec (Quadrature Decoder)
 * RNG (Radom Number Generator)
 * RTC
 * *SAADC*
 * SPI
 * SPIM (Master)
 * SPIS (Slave)
 * Temp (Temperature)
 * Timer
 * TWI (Two Wire Interface, IIC)
 * TWIS (Slave)
 * UART
 * UARTE (Event)
 * WDT (Watch Dog Timer)

More detailed description could be found in [nRF51 Reference Manual](http://infocenter.nordicsemi.com/pdf/nRF51_RM_v3.0.1.pdf) for each entry above, except *italic* ones.
