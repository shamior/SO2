#include "types.h"

struct trap_frame {
    uint64 regs[32];
    uint64 satp;
    // trap_stack aponta para a pilha que vai ser usada pelo kernel para tratar o trap
     // Deve apontar para o final da página alocada para a pilha
     // para se chegar ao final da página com o um ponteiro uint8, basta: trap_stack += 4096
    uint8 * trap_stack;
    uint64 hartid;
    uint64 epc;
};

typedef struct trap_frame TrapFrame;