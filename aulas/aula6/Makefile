PRE = riscv64-linux-gnu-
# PRE = riscv64-unknown-elf-
GCC = $(PRE)gcc
LD = $(PRE)ld
SCRIPT = -Tkernel.ld
QEMU = qemu-system-riscv64
FLAGS+=-mcmodel=medany
# pedantic: gera o diagnóstico descrito pelo padrão C89 da linguagem C
FLAGS += -Wall -Wextra -pedantic -O0 -g
FLAGS += -march=rv64gc -mabi=lp64
FLAGS += -ffreestanding
LDFLAGS += -nostdlib

OBJ = \
	boot.o \
	main.o \
	uart.o \
	printf.o


%.o: %.c
	$(GCC) $(FLAGS) -c -o $@ $<

%.o: %.s
	$(GCC) $(FLAGS) -c -o $@ $<

kernel: $(OBJ)
	$(LD) $(LDFLAGS) $(SCRIPT) -o $@  $(OBJ)

run: kernel
	$(QEMU) -append 'console=ttyS0'  -nographic -serial mon:stdio -smp 4 -machine virt -bios none -kernel kernel
debug: kernel
	$(QEMU) -machine virt -m 128M -gdb tcp::1234 -kernel kernel
clean:
	rm -rf kernel
	rm -rf *.o