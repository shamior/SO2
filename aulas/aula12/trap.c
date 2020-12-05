#include "proc.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "console.h"

void
mtrap(uint64 tval,
        uint64 cause,
        uint64 status,
        TrapFrame *tf)
{
    uint16 icause; // Causa da interrupção (quando o envento é assíncrono).

    if ((long)cause > 0) {
        // Exceções síncronas
        switch(cause) {
            // Falta de página enquanto acessava (fetch) uma instrução
            case 12:
                printf("trap: falta de página (instrução)\n");
                tf->epc = tf->epc + 4;
                break;
            // falta de  página ao carregagar (load) dados da memória
            case 13:
                printf("trap: falta de página (load) CPU#:%d -> epc:%p tval:%p\n" CR, (int) tf->hartid, (uint64 *)tf->epc, (uint64 *)tval);
                tf->epc = tf->epc + 4;
                break;
            // falta de página ao armazenar (store) dados na memória.
            case 15:
                printf("trap: falta de página (store) CPU#:%d -> EPC:%p tval:%p\n"
                        CR,
                        (int) tf->hartid, (uint64 *)tf->epc, (uint64 *)tval);
                tf->epc = tf->epc + 4;
                break;
            // illegal instruction
            case 2:
                printf("trap: Illegal Instruction\n");
                tf->epc = tf->epc + 4;
                break;
            default:
                panic("trap: Exceção não manipulada\n");
                break;
        }

    }
    else {
        // Interrupções (eventos assícronos)
        icause = cause & 0xfff;
        switch(icause) {
            default:
                panic("!!!trap: Caramba, não tratamos esta interrupção.");
                break;
        }

    }
}