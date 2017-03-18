PROJECT_NAME := ble_clds_s110_nRF51822

###
### Makefile and template
###
export OUTPUT_FILENAME
MAKEFILE_NAME := $(MAKEFILE_LIST) #By default, $(MAKEFILE_LIST)=Makefile
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = ../../../../SDK/toolchain/gcc
ifeq ($(OS),Windows_NT)
include $(TEMPLATE_PATH)/Makefile.windows
else
include $(TEMPLATE_PATH)/Makefile.posix
endif

###
### tools
###
MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @  #'@' following the Tab makes the command excuted without print out
endif

# Toolchain commands
CC              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE            := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1)))) 
#$(strip string) is used to adjust format of string so that each two words in the string are sperated by only on blackspace. 
#First one plus the rest where the first one is filtered out, expended recursively until empty string is passed as argument $1

###
### source files and including path
###
#assembly files (startup code) common to all targets
ASM_SOURCE_FILES  = $(abspath ../../../../SDK/toolchain/gcc/gcc_startup_nrf51.s)

#source common to all targets
C_SOURCE_FILES =  $(abspath ../../main.c) 
C_SOURCE_FILES += $(abspath ../../../../SDK/toolchain/system_nrf51.c) 
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/delay/nrf_delay.c) 
#source to use resources (LED&Btn) on a specific board
#C_SOURCE_FILES += $(abspath ../../../BSP/bsp.c)
#C_SOURCE_FILES += $(abspath ../../../bsp/bsp.c)
#C_SOURCE_FILES += $(abspath ../../../bsp/bsp_btn_ble.c)
#peripheral drivers offered in SDK and their IRQ handlers
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/common/nrf_drv_common.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/gpiote/nrf_drv_gpiote.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/clock/nrf_drv_clock.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/rtc/nrf_drv_rtc.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/pstorage/pstorage.c)
#simple application modules and their error handlers
C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/util/app_error.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/timer/app_timer.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/calendar/app_calendar.c)
#C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/button/app_button.c)#use nrf_drv_gpiote operating button actions as interrupt, and use app_timer for delay detection
#source for setting handlers of softdevice, access to ble events
C_SOURCE_FILES += $(abspath ../../../../SDK/softdevice/common/softdevice_handler/softdevice_handler.c)
#BLE basic functionality for data encoding, connection parameters setting, and characteristic, descriptor adding to services 
C_SOURCE_FILES += $(abspath ../../../../SDK/ble/common/ble_advdata.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/ble/common/ble_conn_params.c)
C_SOURCE_FILES += $(abspath ../../../../SDK/ble/common/ble_srv_common.c)
#module to manage advertising with four modes and whitelist
C_SOURCE_FILES += $(abspath ../../../../SDK/ble/ble_advertising/ble_advertising.c)
#source for custom service
#C_SOURCE_FILES += $(abspath ../../../../SDK/ble/ble_services/ble_lbs/ble_lbs.c)
#C_SOURCE_FILES += $(abspath ../../../../SDK/ble/ble_services/ble_BLS/ble_BLS.c)
#C_SOURCE_FILES += $(abspath ../../../../SDK/ble/ble_services/ble_clds/ble_clds.c)
#source for using uart
#C_SOURCE_FILES += $(abspath ../../../../SDK/drivers_nrf/uart/nrf_drv_uart.c)
#C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/uart/app_uart.c)
#C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/uart/retarget.c) 
#app_trace -- a module outputing log through uart
C_SOURCE_FILES += $(abspath ../../../../SDK/libraries/trace/app_trace.c)
#module to manage device information and services context
#C_SOURCE_FILES += $(abspath ../../../../SDK/ble/device_manager/device_manager_peripheral.c)
#sources to support JLink tool -- RTT (Real-Time Terminal)
C_SOURCE_FILES += $(abspath ../../../../RTT/RTT/SEGGER_RTT.c)
C_SOURCE_FILES += $(abspath ../../../../RTT/RTT/SEGGER_RTT_printf.c)


#includes common to all targets
INC_PATHS  = -I$(abspath ../../../../SDK/device)
INC_PATHS += -I$(abspath ../../../../SDK/toolchain)
INC_PATHS += -I$(abspath ../../../../SDK/toolchain/gcc)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/delay)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/hal)
#includes to use resources (LED&Btn) on specific board
INC_PATHS += -I$(abspath ../../../BSP)
#INC_PATHS += -I$(abspath ../../../bsp)
INC_PATHS += -I$(abspath ../../config)
#includes for peripheral drivers offered by SDK
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/config)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/clock)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/rtc)
#INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/uart)
INC_PATHS += -I$(abspath ../../../../SDK/drivers_nrf/pstorage)
#includes for simple applications and their error handler
INC_PATHS += -I$(abspath ../../../../SDK/libraries/util)
INC_PATHS += -I$(abspath ../../../../SDK/libraries/timer)
INC_PATHS += -I$(abspath ../../../../SDK/libraries/calendar)
INC_PATHS += -I$(abspath ../../../../SDK/libraries/button)
#INC_PATHS += -I$(abspath ../../../../SDK/libraries/uart)
INC_PATHS += -I$(abspath ../../../../SDK/libraries/trace)
#headers for softdevice
INC_PATHS += -I$(abspath ../../../../SDK/softdevice/s110/headers)
INC_PATHS += -I$(abspath ../../../../SDK/softdevice/common/softdevice_handler)
#headers for basic ble functionalities
INC_PATHS += -I$(abspath ../../../../SDK/ble/common)
#headers for advertising managing
INC_PATHS += -I$(abspath ../../../../SDK/ble/ble_advertising)
#headers for device information managing
#INC_PATHS += -I$(abspath ../../../../SDK/ble/device_manager)
#headers for ble services
#INC_PATHS += -I$(abspath ../../../../SDK/ble/ble_services/ble_lbs)
#INC_PATHS += -I$(abspath ../../../../SDK/ble/ble_services/ble_BLS)
#INC_PATHS += -I$(abspath ../../../../SDK/ble/ble_services/ble_clds)
#headers for JLink tool -- RTT (Real-Time Terminal)
INC_PATHS += -I$(abspath ../../../../RTT/RTT/)

###
### output settings
###
OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) ) #BUILD_DIRECTORIES are/is the summary of directories used for ouput ordered alphabetically


###
### compiler and linker parameters
###
#flags common to all targets
CFLAGS  = -DNRF51                ### parameter -DXXX for gcc means predefine XXX
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DS110
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror -O3
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums

# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF51
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DS110
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0

###
### targets
###
#default target - first one defined
default: clean nrf51822_xxaa_s110

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf51822_xxaa_s110
	# @make -f Makefile -C /home/babypanda/nRF_IDE/Project/ble_Btn_LED/s110/armgcc/ -e cleanobj        # -f specify script file, -C change work directory, -e = --environment-overrides Give variables taken from the environment precedence over variables from makefiles. 

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf51822_xxaa_s110
	@echo 	flash_softdevice

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf51822_xxaa_s110: OUTPUT_FILENAME := nrf51822_xxaa_s110
nrf51822_xxaa_s110: LINKER_SCRIPT=ble_RTC.ld
nrf51822_xxaa_s110: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<


# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out


## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o

flash: $(MAKECMDGOALS)
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf51  --sectorerase
	nrfjprog --reset
flash_softdevice:
	@echo Flashing: s110_nrf51_8.0.0_softdevice.hex
	nrfjprog --program ../../../../SDK/softdevice/s110/hex/s110_nrf51_8.0.0_softdevice.hex -f nrf51 --chiperase
	nrfjprog --reset
