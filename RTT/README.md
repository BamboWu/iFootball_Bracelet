README.md for the SEGGER RTT Implementation Pack.  
Date: 20 Jan 2017  
  
**Included files:**  
===============  
Root Directory  
  * RTT
    * SEGGER\_RTT.c		- The RTT implementation.
    * SEGGER\_RTT.h		- Header for RTT implementation.
    * SEGGER\_RTT\_Conf.h	- Pre-processor configuration for the RTT implementation.
    * SEGGER\_RTT\_Printf.c	- Simple implementation of printf to write formatted strings via RTT.
  * Syscalls
    * RTT\_Syscalls\_GCC.c	- Low-level syscalls to retarget printf() to RTT with GCC / Newlib.
    * RTT\_Syscalls\_KEIL.c	- Low-level syscalls to retarget printf() to RTT with KEIL/uVision compiler.
