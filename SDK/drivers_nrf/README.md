This directory contains nRF51 peripheral driver files.  
Peripheral drivers work similarly as firmware library in STM32. They are some MACRO defines and functions concerning peripherals. Part of them are located in hal folder.  
Files in delary folder are used for delay, and files in nrf\_soc\_nosd are used when no softdevice is used.   
Besides, more peripherals package their drivers for easier usage in application based on ../libraries/util.They are:  
 * flash
 * clock
 * gpiote (task & event)
 * LPComp (Low Power Comparetor)
 * NVMC (Non-Volatile Memory Controller)
 * PPI (Programmable Peripheral Interconnect)
 * pstorage
 * QDec (Quadrature Decoder)
 * Radio
 * RNG (Radom Number Generator)
 * RTC
 * SDIO
 * SPIM (Master)
 * SPIS (Slave)
 * SWI (SoftWare Interrupt)
 * Timer
 * TWIM (Two Wire Interface Master, IIC)
 * TWIS (Slave)
 * UART
 * UARTE (Event)
 * WDT (Watch Dog Timer)

