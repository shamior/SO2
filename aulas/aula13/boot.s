
.section .data 
.global STACK_START  # Torna visível o início da pilha para o código C
STACK_START: .dword stack_start 
.global STACK_END # Torna visível o fim da pilha para o código C
STACK_END: .dword stack_end
.section .text

.global _start
.equ STACK_SIZE, 0x1000           # tamanho da pilha por CPU (4096 bytes)

_start:
    # setup stacks per hart
    csrr t0, mhartid            # obtém o ID do hart atual
    li a0, STACK_SIZE           # carrega o valor 4096 para a0
    mul t1, t0, a0
    la sp, stack_end                
    sub sp, sp, t1              # define o endereço da pilha para o hart atual

    # Os hards com id != de 0 saltam para park
    csrr a0, mhartid                
    bnez a0, park                                          
    
    # Uso do call (no lugar de j) para permite o código em C 
    # voltar para cá (p/ o endereço park)
    # Para a diferença entre as pseudo-instruções j e call consultar:
    # https://www.imperialviolet.org/2016/12/31/riscv.html
    
    call    entry                  # salta para o código C

park:
    wfi
    j park
