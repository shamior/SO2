#define PAGE_IDX_MASK 0x1FF
#define PAGE_SHIFT 12
#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4) 

#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
// satp tem três campos:
//  - MODE [60-63]
//  - ASID [44-59]
//  - PPN [0-43]
// satp.MODE (satp[60-63]) = 8 para ativar o modo MMU no esquema sv39
#define SATP_SV39 (8L << 60) 
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

static inline void
w_sapt(uint64 x) {
    asm volatile ("csrw satp, %0" : : "r" (x)); 
}

static inline void
sfence_vma()
{
  asm volatile("sfence.vma zero, zero");
}

#define MSTATUS_MPP_MASK (3L << 11) 
#define MSTATUS_MPP_M (3L << 11)
#define MSTATUS_MPP_S (1L << 11)
#define MSTATUS_MPP_U (0L << 11)
#define MSTATUS_MIE (1L << 3)   // habilita interrupções no modo máquina


static inline uint64
r_mstatus()
{
  uint64 x;
  asm volatile("csrr %0, mstatus" : "=r" (x) );
  return x;
}

static inline void 
w_mstatus(uint64 x)
{
  asm volatile("csrw mstatus, %0" : : "r" (x));
}

static inline void 
w_mepc(uint64 x)
{
    asm volatile("csrw mepc, %0" : : "r" (x));
}