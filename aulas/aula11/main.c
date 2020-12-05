#include "types.h"
#include "defs.h"
#include "console.h"
#include "riscv.h"

void test();

static void verify_MIE(){
    register uint64 a5 asm("mstatus");
    printf("%d", a5);
    uint64 NAOTENHOMAISESPERACAS = r_mstatus();
    printf("x = %d\n", NAOTENHOMAISESPERACAS);
    NAOTENHOMAISESPERACAS = NAOTENHOMAISESPERACAS & MSTATUS_MIE;
    printf("As interrupcoes do sistema no modo M esta ");
    if (NAOTENHOMAISESPERACAS){
        printf("habilitada\n");
    }else{
        printf("desabilitada\n");
    }
}


void
main() {
    int c;
    // \uXXXX e \UXXXXXXXX são chamados universal-character-name
    printf(CLEAR_SCREEN CURSOR_UP_LEFT CORAL "\u26F0  尺OS - 尺oncador Operating System (V0.1) \U0001F920\n");
    memory_test();
    verify_MIE();
    // puts("\u26F0  尺OS - 尺oncador Operating System (V0.1) \U0001F920");
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
            case '\r': 
                printf("\r\n"); 
                break;
            case 0x1b: // esc
                // https://en.wikipedia.org/wiki/ANSI_escape_code#Escape_sequences
                // Código ANSI define uma sequencia de três caracteres para teclas como 
                // seta para cima (esc [ A)
                //0x5b == '['
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
    // ativa memória virtual e retorna para a função main no 
    // modo supervisor (S-Mode)  
    kvminit();
    // nunca alcançado 
    return 0; 
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