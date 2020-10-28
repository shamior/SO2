#include <stdio.h>

int adiciona(int a, int b);

int main(int argc, char const *argv[])
{
    int r;
    
    r = adiciona(3, 4);
    printf("Soma:%d\n", r);
    return 0;
}