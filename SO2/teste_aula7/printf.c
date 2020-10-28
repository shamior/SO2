//editei o envio, minha funcao printptr nao estava trabalhando com long sem sinal
//e tambem adicionei no %d e %l a capacidade de imprimir numero negativo


#include "types.h"
#include "defs.h"
#include <stdarg.h>

void puts_backwards(char* str){
    //fiz uma funcao recursiva para imprimir de traz pra frente
	if (*str != '\0'){ //enquanto nao chegar no caractere nulo
		puts_backwards(str + 1); //chama recursivamente
		uartputc(*str); //depois que chegar no nulo, printa na tela
        // de traz para frente
	}
}
static char *digitos = "0123456789abcdef";


void
printlng(long num, int base) {
    char buffer[100]; //bufferzinho
    int resto, pos = 0; //definindo variaveis que irei utilizar
    do{
        resto = num % base;
        num = num / base;
        buffer[pos++] = digitos[resto];
    }while(num != 0); //enquanto nao zerar nosso numero, continua enchendo nosso buffer
    switch (base){ //fiz um switch so para concatenar
        case 2: //quando for binario
            buffer[pos++] = 'b';
            buffer[pos++] = '0';
            break;
        case 16: //ou hexadecimal
            buffer[pos++] = 'x';
            buffer[pos++] = '0';
            break;
    }
    buffer[pos] = '\0'; //caracter nulo para o final da string
    puts_backwards(buffer); //imprime o buffer ao contrario
}

void
printptr(uint64 *ptr){
    int base = 16;
    char buffer[100];
    int resto, pos = 0; 
    do{
        resto = (uint64)ptr % base;
        ptr =(uint64*) ((uint64)ptr / base);
        buffer[pos++] = digitos[resto];
    }while(ptr != 0);
    buffer[pos++] = 'x';
    buffer[pos++] = '0';
    buffer[pos] = '\0';
    puts_backwards(buffer);
}

void
printf(char *s, ...){
    int digit;
    int long_digit;
    va_list ap;
    va_start(ap, s);
    while(*s){ //enquanto nao chegar no caractere nulo
        switch (*s){ //vejo qual caractere
            case '%': //se for %
                s++; //consumo o %
                switch (*s){ //e faco outro switch
                    case 'd': //se for digito
                        digit = va_arg(ap, int);
                        if (digit < 0){
                            digit *= -1;
                            uartputc('-');
                        }
                        printlng(digit, 10);
                        break;
                    case 's': //string
                        printf(va_arg(ap, char*));
                        break;
                    case 'p': //ponteiro
                        printptr(va_arg(ap, uint64*));
                        break;
                    case 'l': //long
                        long_digit = va_arg(ap, long);
                        if (long_digit < 0){
                            long_digit *= -1;
                        }
                        printlng(long_digit, 10);
                        break;
                    case 'x': //hexadecimal
                        printlng(va_arg(ap, int), 16);
                        break;
                    case 'c': //caractere
                        uartputc(va_arg(ap, int));
                        break;
                }
                    
            break;
            default: //se nao for %
                uartputc(*s); //entao coloco o caractere atual no console
                break;
            
        }
        s++; //itero o endereco da string
    }
}
