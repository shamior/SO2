#ifndef DEFS_HEADER
#define DEFS_HEADER
//UART
void uartputc(int);
int uartgetc();
void uartinit();

//print
void printlng(long, int);
void printptr(uint64 *);
void printf(char *, ...);
#endif