#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "riscv.h"
#include "string.h"
#include "console.h"

// Os labels assembly podem ser acessados como uint64 em vez de ponteiros
// neste caso, sempre que quiser acessar o conteúdo das variáveis como
// ponteiro é só fazer um cast (como na chamada a printf p/ imprimir o end.
// da pilha)
// extern uint64* STACK_START;
// extern uint64* STACK_END;

/* O espaço de endereçamento do próprio kernel é virtualizado 
   para efeitos de proteção do código/dados. O espaço será mapeado da seguinte forma:
    - texto, incluindo rodata:  (r-x)
    - dados + pilha + heap:     (rw-)      
    - CLINT
    - PLIC
    - UART (a região mapeada para o dispositivo UART)
   text_end é o primeiro byte depois do fim do texto, servindo como divisor entre as 2 
   primeiras regiões
*/
extern char text_end[]; 
extern char stack_start[]; 
extern char stack_end[]; 

long total_pages;
long alloc_start; 
uint64 *kernel_pagetable;
uint64 *kernel_heap; //Sem uso. Remover

#define HEAP_START  stack_end
#define HEAP_END    (uint8*)(KERNEL_START + MEMORY_LENGTH) 
#define TEXT_END    (uint8*)text_end
#define HEAP_SIZE   ((uint64) HEAP_END - (uint64) HEAP_START)
//Para controle da lista de páginas livres
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

// Converte o endereço de memória addr no seu endereço de página com
// arredondamento para baixo. 
uint64
page_round_down(uint64 addr) {
    return (addr & ~(PAGE_SIZE - 1));
}

// Converte o endereço de memória addr no seu endereço de página com
// arredondamento para cima
uint64 
page_round_up(uint64 addr) {
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

// Retorna o índice (L2, L1 ou L0) do endereço virtual de acordo com o nível
// Formato do VA: |L2 | L1 | L0| offset|, 
// onde L2 = L1 = L0 = 9 bits e offset = 12 bits.
int 
page_idx(uint64 va, int level) {
    return (va >> (PAGE_SHIFT + level * 9)) & PAGE_IDX_MASK;
}

// Retorna o endereço físico (end. de pág) armazenado em uma PTE
// (entrada da tabela de páginas)
// Uma PTE ocupa 64 bits:
// | reservado (10 bits) | PPN (44 bits) | flags (10 bits) | 
uint64
pte2pa(pte_t pte) {
    return (pte >> 10) << PAGE_SHIFT;
}

uint64
pa2pte(uint64 pa) {
    return ((pa >> PAGE_SHIFT) << 10);
}

// Aloca pages páginas da região do HEAP do kernel. 
// Retorna um ponteiro para o primeiro byte da página alocada
void *
alloc(int pages) {
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
        return (uint8 *)(alloc_start + ((uint64)desc - (uint64)HEAP_START) * PAGE_SIZE); 
    }
    return 0;
}

// Buscar o PTE (de L0) correspondente ao VA (uma consulta), ou
// alocar uma página para a tabela de páginas, caso não exista 
// (criação da entrada)
pte_t * 
walk(pte_t *pagetable, uint64 va, int alloc_pg) {
    int level;
    // pagetable aponta sucessivas páginas de tabela de página: L2, L1 e depois L0.
    pte_t *pte;
    // kprintf("VA: %p\n", (uint64*)va);
    // va válidos vão de 0 a MAXVA - 1
    if (va >= MAXVA)
        panic("walk - espaço de endereçamento virtual maior que o permitido");
    for(level = 2; level > 0; level--) {
        /*
            O porquê do '&' antes de pagetable:
            Suponhamos que pagatable começe no endereço 0x10000 e que pagtable[1] == 5555
            pagetable[1]:   Retorna o conteúdo do segundo elemento do vetor pagetable: 5555
            &pagettable[1]: retorna o endereço do segundo elemento do vetor pagetable: 0x1001
         */
        pte = &pagetable[page_idx(va, level)];
        // kprintf("Level %d.  Endereco PTE %p. Índice da página:%d\n", level, pte, page_idx(va, level));
        if (*pte & PTE_V) {
            pagetable = (uint64 *) pte2pa(*pte);
        }
        else {
            if(!alloc_pg || (pagetable = (uint64 *) alloc(1)) == 0) {
                return 0;
            }
            // !!! zerar pagina alocada
            memset(pagetable, 0, PAGE_SIZE);
            *pte = pa2pte((uint64) pagetable) | PTE_V; 
        }
    }
    //kprintf("Level %d.  Endereco PTE %p. Índice da página:%d\n", level, &pagetable[page_idx(va, 0)], page_idx(va, level));

    //pagetable aponta página de tabela de páginas L0
    return &pagetable[page_idx(va, 0)];
}


// Mapeia  páginas sequenciais de endereço virutal para páginas de endereço físico:
//  O endereço virtual (va) é usado para recuperar, via travessia das tabelas de páginas, o PTE_L0 . 
//  O PTE_L0 recebe o endereço físico (pa) 
//  Estes 2 passos são repeditos para sz/PAGE_SIZE páginas 
int 
mappages(uint64 *pagetable, uint64 va, uint64 sz, uint64 pa, int perm) {
    uint64 addr, end;
    pte_t *pte;

    // sz é em bytes e  pode não ser multiplo de PAGE_SIZE
    addr = page_round_down(va); // Obtém end. da página
    end = page_round_up(va+sz);
    while( addr < end) {
        if( (pte = walk(pagetable, va, 1)) == 0) {
            return -1;
        }
        if(*pte & PTE_V) { // A página já está sendo utilizada
            panic("mapppages: remap");
        }
        // Atribui o endereço físico à entrada PTE (L0) do VA
        // Configura permissões (rwx) e bit de validade
        *pte = pa2pte(pa) | perm | PTE_V;
        va += PAGE_SIZE;
        pa += PAGE_SIZE;
        addr += PAGE_SIZE;
    } 
    return 0; // Tudo OK
}

void
pages_init() {
    uint8 * ptr = (uint8 *) HEAP_START;
    int i;
    int reserved_pages; //páginas ocupadas pela lista de descritores

    total_pages = HEAP_SIZE / PAGE_SIZE;
    reserved_pages = total_pages / PAGE_SIZE;
    if(total_pages % PAGE_SIZE) {
        reserved_pages++;
    }
    total_pages -= reserved_pages;
    for (i = 0; i < total_pages; i++) {
        *ptr++ = FREEPG; 
    }
    printf("HEAP START <pages_init>:%p\n", HEAP_START);
    printf("Paginas da lista de desc: %d\n", reserved_pages);
    alloc_start = page_round_up((uint64)HEAP_START + reserved_pages * PAGE_SIZE); 
}

void
memory_init() {
    pages_init();
}

void kvmmap(uint64 va, uint64 pa, uint64 sz, int perm) {
    
    if(mappages(kernel_pagetable, va, sz, pa, perm) != 0) {
        panic("Erro de mapeamento");
    }
}

//Mapeamento de identidade dos seguintes blocos:
//  - Os dois segmentos do kernel: texto e dados+pilha+heap
//  - UART
void kvminit(){
    // A área de heap para uso exclusivo dos objetos do kernel é de 64 *4096 = 256KiB
    kernel_heap = (uint64 *) alloc(64); // Por enquanto não há uso para isso
    // página L2 (o 1º nó) da tabela de páginas
    if((kernel_pagetable = (uint64*) alloc(1)) == 0) {
        panic("Erro ao alocar página L2 para tabela de página");
    }
    memset(kernel_pagetable, 0, PAGE_SIZE);
    
    // A região do código (text + rodata) é de leitura e execução
    kvmmap(KERNEL_START, KERNEL_START, (uint64) text_end - KERNEL_START, PTE_R | PTE_X);
    // A região de dados (dados + pilha + heap) é de leitura e escrita        
    kvmmap((uint64) text_end, (uint64) text_end, (KERNEL_START + MEMORY_LENGTH - (uint64) text_end), PTE_R|PTE_W);
    // Precisamos escrever na região de memória onde o UART está maepado
    kvmmap(UART, UART, PAGE_SIZE, PTE_R | PTE_W);

    //!!! no momento estes mapeamentos são irrelevantes porque a inicialização de dispositivos e
    // !!! tratamento de interrupções se dá no modo-M onde o MMU está desativado
    // OS CSR ocupam 0x10000 de memória
    kvmmap(CLINT, CLINT, 0x10000, PTE_R | PTE_W);
    kvmmap(PLIC, PLIC, 0x400000, PTE_R | PTE_W);
    // os 8 dispositivos que o nosso SO reconhece estão separados por endereços de 0x1000 (4096)
    // começando em 0x10001000 
    // kvmmap(MMIO_VIRTIO_START, MMIO_VIRTIO_START, 0x8000, PTE_R | PTE_W);

    
    //  pte = walk(kernel_pagetable, CLINT_MTIMECMP(0), 0);
    // kprintf("CLINT_MTIMECMP - VA: \t\t%p PA:%p PTE_FLAGS:%d\n", CLINT_MTIMECMP(0), 
    // (uint64*)pte2pa(*pte),  pte_flags(*pte));
    
    /*
    Para entrar no modo-S vamos executar os seguintes passos:
        1. Configurar o SATP
        2. Habilitar MSTATUS.MPP para Modo-S
        3. Configura endereço de retorno para a função main em main.c: mepc = &main
        4. Invalidar todo o TLB: sfence.vma
        5. mret
        3. (não impl.) Desabilitar todas as interrupções (nesta versão apenas): MSTATUS.MPIE = 0
    */

    // Ativar modo Sv39 e informar o endereço da tabela raiz (L2)
    w_sapt(MAKE_SATP(kernel_pagetable));
    // Configura MSTATUS para que a hart possa entrar no modo S-Mode  
    uint64 x = r_mstatus();
    // Zera apenas o campo MSTATUS.MPP (2 bits)
    x = x & ~MSTATUS_MPP_MASK;
    // Atribui o privilégio S-Mode ao campo MSTATUS.MPP
    x = x | MSTATUS_MPP_S;
    w_mstatus(x);

    // Desabilita todas as interrupções porque o kernel ainda não é capaz
    // de trata-las: MSTATUS.MPIE = 0. (quando mret executar MIE vai recebe MPIE)
    // Como todas as interrupções são atendidas no modo-M (a não ser que haja delegações),
    // se MIE == 0, elas serão ignoradas
    // uint64 reg = r_mstatus();
    // reg = reg & ~(1L << 7);
    // w_mstatus(reg);

    //O fluxo de execução retorna para a função main em main.c
    w_mepc((uint64) main);

    //invalida as entradas da TLB 
    sfence_vma(); 
    asm volatile("mret");
}

void
memory_test() {
    char* p =  alloc(1);
    *p = 'O';
    *(p+1) = 'i';
    *(p+2) = '\n';
    char *p2 = alloc(2);
    printf("End. de p:\t%p\n", p);
    printf("Conteúdo de p:\t%s\n", p);
    printf("End. de p2:\t%p\n", p2);
    uint8 *desc;
    desc = (uint8*) HEAP_START;
    kfree(p2);
    printf("DESCRITOR PÁG.\tSTATUS (0-OCUPADA  1-LIVRE  2-OCUPADA/ÚLTIMA)\n");
    for(int i = 0; i < 10; i ++) { //Os 5 primeiros descritores
        printf("%d\t  \t%d\n", i, *(desc+i)); 
    }
    printf("TEXT_END:\t%p\n", TEXT_END);
    printf("Início pilha:\t%p\n", stack_start);
    printf("Fim da pilha:\t%p\n", stack_end);
    printf("HEAP_START:\t%p\n", HEAP_START);
    printf("HEAP_END:\t%p\n", HEAP_END);
    printf("HEAP_SIZE:\t%p\n", (uint64*) HEAP_SIZE);
    printf("alloc_start:\t%p\n", (uint64*) alloc_start);
    printf(LIGHT_SEA_GREEN);
    printf(" --- PAGINAÇÃO (endereços virtuais e físicos dos segmentos) ---\n");
    pte_t *pte = walk(kernel_pagetable, KERNEL_START, 0);
    printf("TEXT_START\t\tVA: %p\tPA:%p\n", KERNEL_START, (uint64*)pte2pa(*pte));
    pte = walk(kernel_pagetable, (uint64) text_end, 0);
    printf("DATA_START\t\tVA:%p\tPA:%p\n", text_end, (uint64*)pte2pa(*pte));
    pte = walk(kernel_pagetable, (uint64) stack_start, 0);
    printf("KERNEL STACK_START\tVA:%p\tPA:%p\n", stack_start, (uint64*)pte2pa(*pte));
    pte = walk(kernel_pagetable, (uint64) HEAP_START, 0);
    printf("HEAP_START\t\tVA: %p\tPA:%p\n", HEAP_START, (uint64*)pte2pa(*pte));
    printf(CR);
}



void kfree(void *ptr){
    //verifico se o endereco esta dentro do heap
    if ((uint64*)ptr >= (uint64*)HEAP_START && (uint8*)ptr <= HEAP_END){
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