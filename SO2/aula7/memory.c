#include "types.h"
#include "defs.h"
#include "memlayout.h"

extern uint64* STACK_START;
extern uint64* STACK_END;

extern char text_end[]; 
long total_pages;
long alloc_start; 

#define HEAP_START  STACK_END
#define HEAP_END    (uint8*)(KERNEL_START + MEMORY_LENGTH) 
#define TEXT_END    (uint8*)text_end
#define HEAP_SIZE   ((uint64) HEAP_END - (uint64) HEAP_START)
#define FREEPG 0x1 // A página está livre
#define LASTPG 0x2 // A página é a última do bloco alocado

int
free_page(uint8 desc) {
    // A página livre tem o bit 0 = 1
    if (desc & FREEPG) 
        return 1;
    else
        return 0;
}

int
last_page(uint8 desc) {
    // A última página do bloco alocado tem o bit 1 = 1
    if (desc & LASTPG)
        return 1;    
    else
        return 0;
}

void
set_free_page_flag(uint8 *desc, uint8 freedom) {
    if (freedom)
        *desc = *desc | 0x01;
    else {
        *desc = *desc & (0xFF << 1); // 0xFE;
    }
}

void
set_last_page_flag(uint8 *desc, uint8 last) {
    if (last) {
        *desc = *desc | (1 << 1); // bit 1 = 1
    }
    else {
       *desc = *desc & ((0xFF << 2) | 1); // (0xFD)  bit 1 = 0
    }
}

/* 
Converte o endereço de memória addr no seu endereço de página com
 arredondamento para cima
*/
uint64 page_round_up(uint64 addr) {
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

uint64 page_round_down(uint64 addr) {
    return (addr & ~(PAGE_SIZE - 1));
}


void verifica_descritores(int qtd){
    int i;
    uint8* desc = HEAP_START;
    for (i = 0; i < qtd; i++){
        if (last_page(*(desc + i))){
            printf("Ultima Pagina\n");
        } else {
            printf("Pagina Normal\n");
        }
    }
}


void *
alloc(int pages) {
    // início da lista dos descritores de página
    uint8 * ptr = (uint8 *) HEAP_START;
    int i;
    int count = 0;
    uint8 * desc = 0; // O descritor da primeira página do bloco alocado
    
    if (pages == 0)
        return 0;
    for(i = 0; i < total_pages; i++) {
        ptr = (uint8 *) HEAP_START + i;
        if(free_page(*ptr)) {
            if (count == 0) 
                desc = ptr;
            count++;
        }
        else {
            count = 0;
            desc = 0;
        }
        if (count == pages) break;
    }
    if (count < pages)
        desc = 0;
    if (desc != 0) {
        for (i = 0; i < pages; i++) {
            set_free_page_flag(desc+i, !FREEPG);
            set_last_page_flag(desc+i, !LASTPG);
        }
        set_last_page_flag(desc + (i-1), LASTPG);
        printf("Páginas alocadas:%d\n", i);
        return (uint8 *)(alloc_start + ((uint64)desc - (uint64)HEAP_START) * PAGE_SIZE); 
    }
    return 0;
}

void
pages_init() {
    uint8 * ptr = (uint8 *) HEAP_START;
    int i;
    int reserved_pages; //páginas ocupadas pela lista de descritores

    total_pages = HEAP_SIZE / PAGE_SIZE;
    printf("total de paginas: %d\n", total_pages);
    reserved_pages = total_pages / PAGE_SIZE;
    if(total_pages % PAGE_SIZE) {
        reserved_pages++;
    }
    total_pages -= reserved_pages;
    for (i = 0; i < total_pages; i++) {
        *ptr++ = FREEPG; 
    }
    printf("Paginas reservadas: %d\n", reserved_pages);
    alloc_start = page_round_up((uint64)HEAP_START + reserved_pages * PAGE_SIZE); 
}

void
memory_init() {
    pages_init();
    char* p =  alloc(1);
    printf("Ptr de alloc(1):%p\n", p);
    *p = 'O';
    *(p+1) = 'i';
    *(p+2) = '\n';
    char *p2 = alloc(2);
    //!! string literal. p aponta para a seção de texto ou a string é copiada para o 
    //!! heap e então seu endereço atribuído a p?
    p = "Ola\n";
     printf("Ptr (strg literal):%p\n", p);   
    uint8 *desc;
    desc = (uint8*) HEAP_START;
    for(int i = 0; i < 10; i ++) {
        printf("Descritor %d - %d\n", i, *(desc+i));
    }
    printf("TEXT_END:\t%p\n", TEXT_END);
    printf("Início pilha:\t%p\n", STACK_START);
    printf("Fim da pilha:\t%p\n", STACK_END);
   
    printf("HEAP_START:\t%p\n", HEAP_START);
    printf("HEAP_END:\t%p\n", HEAP_END);
    printf("HEAP_SIZE:\t%p\n", (uint64*) HEAP_SIZE);
    printf("total_pages:\t%d\n",total_pages);
    printf("alloc_start:\t%p\n", (uint64*) alloc_start);
    printf("Conteúdo do heap:%s\n", p);
   
    printf("Ptr de alloc(2):%p\n", p2);
    for(int i = 0; i < 10; i ++) {
        printf("Descritor %d - %d\n", i, *(desc+i));
    }

    verifica_descritores(20);



}

void kfree(void *ptr){
    //verifico se o endereco esta dentro do heap
    if (((uint64*)ptr) >= HEAP_START && (uint8*)ptr <= HEAP_END){
        uint8* desc;
        uint64 page_number;
        page_number =  ((uint64)ptr - alloc_start)/PAGE_SIZE;
        //ptr - alloc_start pega o deslocamento em bytes da pagina alocada
        //depois eh dividido por PAGE_SIZE para dar o numero da pagina
        desc = (uint8*) (page_number + (uint64)HEAP_START);
        //com o numero da pagina em maos so precisamos deslocar esse numero da pagina
        //em relacao ao HEAP_START para obtermos o descritor da pagina

        //uma simples verificacao para confirmar se o usuario nao passou uma pagina livre
        if (!free_page(*desc)){
            while(!last_page(*desc)){
                set_free_page_flag(desc, 1);
                desc++;
            }
            set_last_page_flag(desc, 0);
            set_free_page_flag(desc, 1);
        }
    }
}