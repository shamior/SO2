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

void puts(char*);

void panic(char *);

//memoria
void memory_init();
void kfree(void *);


#endif