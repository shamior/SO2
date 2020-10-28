.global _start

.equ STACK_SIZE, 4096           # tamanho da pilha

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

    j    entry                  # salta para o código C

park:
    wfi
    j park

.skip STACK_SIZE * 4            
stack_end: