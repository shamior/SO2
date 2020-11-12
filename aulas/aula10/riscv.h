#define PAGE_IDX_MASK 0x1FF
#define PAGE_SHIFT 12
#define PTE_V (1L << 0)
// Tamanho máximo que permitiremos para a memória virtual
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1)) // 256GiB