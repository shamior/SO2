#include "types.h"
#include "memlayout.h"

//https://www.lammertbies.nl/comm/info/serial-uart
#define RHR 0 // receive holding register (for input bytes)
#define THR 0 // transmit holding register (for output bytes)
#define IER 1 // interrupt enable register
#define FCR 2 // FIFO control register
#define ISR 2 // interrupt status register
#define LCR 3 // line control register
#define LSR 5 // line status register

void write_reg(uint64, char);
char read_reg(uint64);

void
uartinit() {
    //Configura registrador LCR (Line Control Register) para permitir
    // comprimento de palavra de 8 bits. Isto se faz setando os bits 0 e 1.
    write_reg(LCR, 0x3);
    // Habilita buffer FIFO de entrada e saída para os bytes.
    // Seta-se o bit 0 do registrador FCR (Fifo Control Register)
    write_reg(FCR, 0x1);
    // Habilita interrupção p/ o buffer de recepção.
    write_reg(IER, 0x1);
}

char
read_reg(uint64 reg) {
    char c;
    volatile uint8 *base = (uint8 *)UART;
    c = *(base + reg);
    return c;
}

void
write_reg(uint64 reg, char c) {
    volatile uint8 *base = (uint8 *)UART;
    *(base + reg) = c;
}

void
uartputc(int c) {

    while((read_reg(LSR) & (1 << 5)) == 0)
    ;
    write_reg(THR, c);
}

int
uartgetc() {
    int c;

    c = read_reg(LSR);
    if ((c & 0x01) == 0) {
        //Nâo há dados para ler
        return -1;
    }
    else {
        c = read_reg(RHR);
        return c;
    }
}