K = src/kernel
U = src/user
B = build

TOOLPREFIX = riscv64-unknown-elf-

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2 -MD -mcmodel=medany 
CFLAGS += -fno-common -nostdlib -Wno-main -fno-builtin -I. -fno-stack-protector 
CFLAGS += -no-pie -fno-pie -ffreestanding -mno-relax

LDFLAGS = -z max-page-size=4096

KSRCS = $(notdir $(wildcard $K/*.c))
KSRCS += $(notdir $(wildcard $K/*.S))
KOBJS = $(KSRCS:.c=.o)
KOBJS := $(KOBJS:.S=.o)
KOBJS := $(patsubst %.o,$B/%.o,$(KOBJS))

$B/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$B/%.o: $K/%.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel: $(KOBJS) $K/kernel.ld 
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o kernel $(KOBJS)
	$(OBJDUMP) -d kernel > kernel.asm


clean: 
	rm -f */*.o */*.d */*.asm kernel


# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

ifndef CPUS
CPUS := 2
RAM :=  256M
endif

QEMU = qemu-system-riscv64

QEMUOPTS = -machine virt -bios none -kernel kernel -m $(RAM) -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
# QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
# QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu: kernel
	$(QEMU) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: kernel .gdbinit fs.img
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

