K = kernel
U = user
B = build

OBJS =	$K/boot.o \
		$K/init.o \
		$K/main.o \
		$K/spinlock.o \
		$K/uart.o \
		$K/string.o	\
		$K/plic.o \
		$K/disk.o \
		$K/sched.o

TOOLPREFIX = riscv64-unknown-elf-

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2 -MD -mcmodel=medany 
CFLAGS += -fno-common -nostdlib -Wno-main -fno-builtin -I. -fno-stack-protector 
CFLAGS += -no-pie -fno-pie


$K/kernel: $(OBJS) $K/kernel.ld 
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS)
	$(OBJDUMP) -d $K/kernel > kernel.asm


clean: 
	rm -f */*.o */*.d */*.asm


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

QEMUOPTS = -machine virt -bios none -kernel $K/kernel -m $(RAM) -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
# QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
# QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu: $K/kernel
	$(QEMU) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: $K/kernel .gdbinit fs.img
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

