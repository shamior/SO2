.section .text

.global adiciona #define o tipo do simbolo adiciona como uma função
.type adiciona, @function 

adiciona:
    add a0, a0, a1
    ret
