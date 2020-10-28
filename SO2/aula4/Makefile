GCC = riscv64-linux-gnu-gcc
#GCC = riscv64-unknown-elf-gcc
CFLAGS+= -mcmodel=medany
QEMU = qemu-system-riscv64
kernel: main.c boot.s
	$(GCC) $(CFLAGS) -g -march=rv64gc -mabi=lp64 -Tkernel.ld -o kernel -nostdlib -ffreestanding boot.s main.c
run: kernel
	$(QEMU) -append 'console=ttyS0' -nographic -serial mon:stdio -smp 4 -machine virt -bios none -kernel kernel		
#debug: kernel
	#$(QEMU) -machine virt -m 128M -gdb tcp::1234 -kernel kernel
clean: 
	rm -rf kernel
