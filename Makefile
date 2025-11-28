# Toolchain
OS := $(shell uname -o)
ifeq ($(OS),GNU/Linux)
CROSS     = riscv64-elf-
else
CROSS     = riscv64-unknown-elf-
endif
CC        = $(CROSS)gcc
AS        = $(CROSS)as
LD        = $(CROSS)ld
OBJCOPY   = $(CROSS)objcopy
OBJDUMP   = $(CROSS)objdump

# Directories
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

CFLAGS    = -march=rv64imac_zicsr -mabi=lp64 \
            -mcmodel=medany \
            -ffreestanding -nostdlib -nostartfiles \
            -Wall -Wextra -O2 -fno-builtin \
            -I./include \
            -g

ASFLAGS   = -march=rv64imac_zicsr -mabi=lp64

LDFLAGS   = -T linker.ld -nostdlib

# Source files
SRC_C     = core/kernel.c core/mem_util.c core/page_manager.c core/kprintf.c arch/riscv/sbi.c
SRC_S     = arch/riscv/boot/start.S

# Object files (in build directory)
OBJ       = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_C)) \
            $(patsubst %.S,$(OBJ_DIR)/%.o,$(SRC_S))

# Output files
KERNEL_ELF  = $(BUILD_DIR)/kernel.elf
KERNEL_BIN  = $(BUILD_DIR)/kernel.bin
KERNEL_DUMP = $(BUILD_DIR)/kernel.dump

# Targets
all: $(KERNEL_ELF) $(KERNEL_BIN) $(KERNEL_DUMP)

# Create build directories
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.S | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJ) linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(OBJ) -o $@

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_DUMP): $(KERNEL_ELF)
	$(OBJDUMP) -D $< > $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

qemu: $(KERNEL_ELF)
	qemu-system-riscv64 -machine virt -bios default -nographic -kernel $(KERNEL_ELF)

qemu_debug: $(KERNEL_ELF)
	qemu-system-riscv64 -machine virt -bios default -nographic -kernel $(KERNEL_ELF) -S -gdb tcp::1234

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all qemu clean
