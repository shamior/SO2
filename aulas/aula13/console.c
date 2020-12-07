#include "types.h"
#include "defs.h"

void console_handler() {
    int c;
    c = uartgetc();
    if (c == -1)
        return;
    switch(c) {
        case 127:
            uartputc('\b');
            uartputc(' ');
            uartputc('\b');
            break;
        case '\r':
            //Dependendo do console emulado pelo qemu, eu recebo '\r', '\n' ou '\r\n'
            // O compilador convert  '\b' e '\n' em representação binária apropriada.
            // 1 byte para cada em vez de 1 byte para '\' e outro para 'r'.
            printf("\r\n");
            break;
        case 0x1b: // esc
            // https://en.wikipedia.org/wiki/ANSI_escape_code#Escape_sequences
            // Código ANSI define uma sequencia de três caracteres para teclas como
            // seta para cima (esc [ A)
            //0x5b == '['
            if( (c = uartgetc()) == 0x5B) {
                if((c = uartgetc()) == -1)
                    return;
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