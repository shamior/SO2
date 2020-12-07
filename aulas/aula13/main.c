#include "types.h"
#include "defs.h"
#include "console.h"
#include "riscv.h"
#include "memlayout.h"
#define ever (;;)

extern void mvector(void); // Rotina q salva os registradores em assembly
extern void msg_syscall(int);

// No final de entry() o sistema salta para cá no modo-S
void
main() {
    // \uXXXX e \UXXXXXXXX são chamados universal-character-name
    printf(CLEAR_SCREEN CURSOR_UP_LEFT CORAL "\u26F0  尺OS - 尺oncador Operating System (V0.1) \U0001F920\n");
    memory_test();
    msg_syscall(10);
    //for ever;
}

// boot.s salta para cá no modo máquina (M-Mode)
void 
entry() {
    // gerenciamento do heap
    memory_init();
    printf("-->Valor de MTIME: %d\n", *(uint64*) CLINT_MTIME);
    printf("-->Valor de CLINT_MTIME: %p\n", (uint64*) CLINT_MTIME);

    //O fluxo de execução retorna para a função main em main.c
    w_mepc((uint64) main);
    // Configura o vetor de interrupções.
    // Quando há uma interrupção/exceção o fluxo é desviado para
    // o endereço presente no registrador mtvec
    uint64 x = (uint64) mvector;
    printf("Endereço do vetor de interrupção:%p\n", mvector);
    w_mtvec(x);
    // ativa memória virtual 
    kvminit();
    // Habilita interrupções
    // configura mstatus.MPIE == 1 para que as interrupções sejam habilitadas 
    // quando o sistema voltar para o modo supervisor (mstatus.MIE <- mstatus.MPIE)
    x = r_mstatus();
    x = x | (1 << 7);
    w_mstatus(x);
    // Habilita interrupção do temporizador
    x = r_mie();
    x = x | (1 << 7); // MTIE
    x = x | (1 << 11); // MEIE (Interrupções externas)
    w_mie(x);
    // ~ 1 interrupção por segundo
    uint64 *mtimecmp = (uint64 *) CLINT_MTIMECMP(0);
    uint64 *mtime = (uint64 *) CLINT_MTIME;
    printf("Valor de MTIME: %d\n", *mtime);
    *mtimecmp = *mtime + 10000000;
    printf("Valor de CLINT_MTIMECMP: %p\n", CLINT_MTIMECMP(0));
    uartinit();
    printf("UART Inicializado\n");
    plic_init();
    printf("PLIC inicializado\n");
    asm volatile("mret"); // retorna para a função main no modo-S
}

void
test() {
    // printf("Fim bss:%p\n", (uint64*)bss_end);
    // int per = perimetro(3, 3, 4, 5);
    // printf("Perímetro triângulo:%d\n", per);
    // per = perimetro(4, 10, 10, 10, 10);
    // printf("Perímetro retângulo:%d\n", per);
    // printf("Perímetro Pentágono:%d\n", perimetro(5, 6, 3, 3, 4, 4));
    // printf("Ponteiro:%p\n", (uint64*)4096);
}