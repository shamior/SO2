a.out: somamain.c soma.s
	riscv64-linux-gnu-gcc -static somamain.c soma.s

clean:
	rm -f a.out
	rm -f *.o
	rm -f *.cow

run: a.out
	/usr/bin/riscv64-linux-gnu/bin/spike -m128 /usr/bin/riscv64-linux-gnu/bin/pk a.out

getcow:
	wget https://raw.githubusercontent.com/Virviil/cow.rs/master/test/hello_world.cow