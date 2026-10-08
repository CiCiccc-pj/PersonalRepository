######################################
# STM32F103C8Tx -- arm-none-eabi-gcc 构建
#
# 用法:
#   make          编译并生成 build/bishe.elf .hex .bin
#   make clean    清理
#
# 注意: 本机 gcc 未附带 newlib，因此:
#   - 编译加 -ffreestanding（使用 GCC 自带 stdint.h）
#   - 链接用 -nostdlib，只链接 libgcc
######################################

TARGET    = bishe
BUILD_DIR = build

######################################
# 工具链
######################################
PREFIX = arm-none-eabi-
CC     = $(PREFIX)gcc
AS     = $(PREFIX)gcc -x assembler-with-cpp
CP     = $(PREFIX)objcopy
SZ     = $(PREFIX)size

######################################
# 源文件
######################################
C_SOURCES = \
Core/Src/main.c \
Core/Src/gpio.c \
Core/Src/usart.c \
Core/Src/dht11.c \
Core/Src/stm32f1xx_it.c \
Core/Src/stm32f1xx_hal_msp.c \
Core/Src/system_stm32f1xx.c \
Core/Src/syscalls.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c

ASM_SOURCES = Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s

######################################
# 宏定义与头文件路径
######################################
C_DEFS = -DUSE_HAL_DRIVER -DSTM32F103xB

C_INCLUDES = \
-ICore/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include

######################################
# 编译 / 链接选项
######################################
MCU     = -mcpu=cortex-m3 -mthumb
OPT     = -Og -g3

CFLAGS   = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -ffreestanding -fdata-sections -ffunction-sections
ASFLAGS  = $(MCU) $(OPT) -Wall -ffreestanding -fdata-sections -ffunction-sections

LDSCRIPT = STM32F103C8Tx_FLASH.ld

LD_FLAGS = $(MCU) -nostdlib -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
LD_LIBS  = -lgcc

######################################
# 目标文件
######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LD_FLAGS) $(LD_LIBS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
