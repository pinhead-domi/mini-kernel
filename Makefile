# Toolchain
OS := $(shell uname -o)
ifeq ($(OS),GNU/Linux)
CROSS     = riscv64-none-elf-
else
CROSS     = riscv64-unknown-elf-
endif
CC        = $(CROSS)gcc
CXX				= $(CROSS)g++
AS        = $(CROSS)as
LD        = $(CROSS)ld
OBJCOPY   = $(CROSS)objcopy
OBJDUMP   = $(CROSS)objdump

# Directories
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

CFLAGS    = -march=rv64imac_zicsr -mabi=lp64 \
            -mcmodel=large \
            -ffreestanding -nostdlib -nostartfiles \
            -Wall -Wextra -fno-builtin \
            -I./include \
            -g

CXXFLAGS    = $(CFLAGS) -fno-exceptions -fno-rtti -fno-threadsafe-statics -fpermissive

ASFLAGS   = -march=rv64imac_zicsr -mabi=lp64

LDFLAGS   = -T linker.ld -nostdlib

# Source files
SRC_C     = core/memory.c core/mem_util.c arch/riscv/boot/setup_paging.c core/page_manager.c core/kprintf.c arch/riscv/sbi.c core/icxxabi.c
SRC_S     = arch/riscv/boot/start.S arch/riscv/trap.S
SRC_CXX   = core/kernel.cpp core/kernel_allocator.cpp

# Object files (in build directory)
OBJ       = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_C)) \
            $(patsubst %.S,$(OBJ_DIR)/%.o,$(SRC_S)) \
						$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_CXX))

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

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
