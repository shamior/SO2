#include "types.h"
#include "memlayout.h"

//Habilita interrupção de número irq. Ex.: UART é a interrupção 10
// Cada interrupção é 1 bit da word de 32 bits
void
plic_enable(int irq){
    uint32 * enables = (uint32 *) PLIC_ENABLE;
     *enables = *enables | (1 << irq);
}

uint32
plic_get_enable(){
    return *(uint32 *) PLIC_ENABLE;
}

//Define prioridade para interrupção
// O 'prio & 7' garante que a maior prioridade seja 7
void
plic_priority(uint32 irq, uint8 prio) {
    * (uint32*)(PLIC_PRIORITY + irq * 4) = prio & 7;
}

uint32
plic_get_priority(uint32 irq) {
   return *((uint32*)(PLIC_PRIORITY) + irq);
}
// Define limiar para as interrupções
void
plic_threshold(uint8 thresh) {
   * (uint32 *) PLIC_THRESHOLD = thresh & 7;
}

uint32
plic_get_threshold() {
   return * (uint32 *) PLIC_THRESHOLD;
}
// Obtém o número da interrupção que gerou o evento
uint32
plic_claim() {
    return  *(uint32 *) PLIC_CLAIM;
}
// Interrupção foi atendida pelo kernel
void
plic_complete(uint32 irq) {
    * (uint32 *) PLIC_CLAIM = irq;
}

void
plic_init() {
    // Todas as interrupções com prioridade menor ou igual ao
    // threshold serão mascaradas. O threshold máximo é 7.
    // No caso, todas as interrupções são aceitas
    plic_threshold(0);
    plic_enable(UART_IRQ);
    plic_priority(UART_IRQ, 1);
}