#include "types.h"
#include "defs.h"
#include "console.h"
#include "riscv.h"

// Rotina em assembly p/ salvar os registradores
extern void mvector(void);

void
main() {
    int c;
    int *ponteru = alloc(1);
    // \uXXXX e \UXXXXXXXX são chamados universal-character-name
    printf(CLEAR_SCREEN CURSOR_UP_LEFT CORAL "\u26F0  尺OS - 尺oncador Operating System (V0.1) \U0001F920\n");
    memory_test();
    ponteru[2562547752] = 8;
    printf("%d\n", ponteru[2562547752]);
    r_mstatus();
    for(;;) {
        c = uartgetc();
        if (c == -1)
            continue;
        switch(c) {
            case 127:
                uartputc('\b');
                uartputc(' ');
                uartputc('\b');
                break;
            case '\r': //Dependendo do console emulado pelo qemu, eu recebo '\r', '\n' ou '\r\n'
                printf("\r\n");
                break;
            case 0x1b: // esc
                if( (c = uartgetc()) == 0x5B) {
                    if((c = uartgetc()) == -1)
                        continue;
                    switch(c) {
                        case 'A':
                            printf("Seta para cima ");
                            break;
                        case 'B':
                            printf("Seta para baixo ");
                            break;
                        default:
                            printf("Outra tecla ANSI ");
                            uartputc(c);
                            uartputc(' ');
                    }
                }
                break;
            default:
                uartputc(c);
        }
    }

}

// boot.s salta para cá no modo máquina (M-Mode)
int
entry() {
    // gerenciamento do heap
    memory_init();
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
    asm volatile("mret"); // retorna para a função main no modo-S
}