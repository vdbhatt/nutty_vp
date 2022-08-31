ARCH = rv32i
CCPATH = /opt/riscv/bin
CROSS = riscv32-unknown-elf
CC = $(CCPATH)/$(CROSS)-gcc
AS = $(CCPATH)/$(CROSS)-as
LD = $(CCPATH)/$(CROSS)-ld
OC = $(CCPATH)/$(CROSS)-objcopy
OD = $(CCPATH)/$(CROSS)-objdump
CPP = $(CCPATH)/$(CROSS)-cpp

BUILD_DIR = $(shell pwd)/build

PROJ   =   nutty

CCFLAGS = -Wall  -O0 -march=$(ARCH) -mabi=ilp32 -D__RISCV__ -DARCH="\"$(ARCH)\""
ASFLAGS = -march=$(ARCH)
LDFLAGS = -Tlink.ld -Map=$(BUILD_DIR)/$(PROJ).map -m elf32lriscv # -Ttext=0
CPFLAGS = -P
OCFLAGS = -O binary
ODFLAGS = -d


SRCS = main.c

ASMS =  $(foreach f, $(subst .c,.s,$(SRCS)), $(BUILD_DIR)/$(f))
OBJS =  $(foreach f, $(subst .c,.o,$(SRCS)), $(BUILD_DIR)/$(f))

OBJS += $(BUILD_DIR)/start.o

FIRMWARE_IMAGE = $(BUILD_DIR)/$(PROJ).rom  $(BUILD_DIR)/$(PROJ).ram

all: $(BUILD_DIR) $(FIRMWARE_IMAGE)

$(BUILD_DIR)/%.s : %.c
	$(CC) $(CCFLAGS) -S $< -o $@

$(BUILD_DIR)/%.o : $(BUILD_DIR)/%.s
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/start.o : startup.S
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(PROJ).elf : $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	$(OD) $(ODFLAGS) $@ > $(BUILD_DIR)/$(PROJ).lst
	$(OC) -O binary  $@  $(BUILD_DIR)/$(PROJ).elf.bin

$(BUILD_DIR)/$(PROJ).rom : $(BUILD_DIR)/$(PROJ).elf
	$(OC) $(OCFLAGS) $< $(BUILD_DIR)/$(PROJ).text --only-section .text*
	hexdump -ve '1/4 "%08x\n"' $(BUILD_DIR)/$(PROJ).text >$@
	wc -l $@

$(BUILD_DIR)/$(PROJ).ram : $(BUILD_DIR)/$(PROJ).elf
	$(OC) $(OCFLAGS) $< $(BUILD_DIR)/$(PROJ).data --only-section .*data*
	hexdump -ve '1/4 "%08x\n"' $(BUILD_DIR)/$(PROJ).data >$@
	wc -l $@


clean:
	rm -rf $(BUILD_DIR)