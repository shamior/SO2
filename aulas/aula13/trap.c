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
    int syscall_number;
    uint64 *mtimecmp = (uint64*)CLINT_MTIMECMP(0);
    uint32 irq;

    if ((long)cause > 0) { 
        // Exceções síncronas
        switch(cause) {
            // syscall vinda do modo-S. Ou seja, a cpu estava no modo-S quando invocou a syscall
            case 9:
                //regs[16]: a7 (o número da syscall para diferenciação entre elas) 
                //regs[9]: a0 (o primeiro parâmetro)
                syscall_number = tf->regs[16];
                if (syscall_number == 1) {
                    printf("Olá da System Call de número %d. Parâmetro: %d\n", syscall_number, tf->regs[9]);
                    printf("Valor de MTIME: %d\n", *(uint64 *) CLINT_MTIME);
                }
                tf->epc = tf->epc + 4;
                break;
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
            default:
            printf("exceção %d\n", cause);
                panic("trap: Exceção não manipulada");
                break;
        }  

    }
    else {
        // Interrupções (eventos assícronos)
        icause = cause & 0xfff;
        switch(icause) {
            case 7: 
                // A interrupção do temporizador sempre é atendida pelo modo-M
                // (não há como delegar para o modo-S)
                printf("\u23F0 Interrupção do temporizador  Modo-M\n");
                *mtimecmp = *(uint64 *) CLINT_MTIME + 10000000;
                break;
            case 11:
                // Obtém  a IRQ que causou a interrupção
                irq = plic_claim();
                switch(irq) {
                    case UART_IRQ: // teclado
                        console_handler();
                        break;
                    default:
                        printf("Interrupção %d  é desconhecida\n", irq);
                        break;
                }
                plic_complete(irq); // IRQ atendida
                break;
            default:
                panic("!!!trap: Caramba, não tratamos esta interrupção.");    
                break;
        }

    }
}