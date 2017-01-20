This directory contains files related to UART, offering functions for application program to use UART.  
And there are two versions of uart application functions: app\_uart\_fifo.c uses FIFO to buffer, while app\_uart.c does not.  
retarget.c is used to retarget printf to UART port, which is a useful feature for debugging!  
