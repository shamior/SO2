/* um script mínimo para o compilar o kernel 'Hello World' 
A seção de texto começa no endereço 0x80000000 de acordo com o que espera 
a márquina virtual RISC-V `virt` do Qemu
*/

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
    . = 0x80000000;
    /* seção de código */
    .text :
    {
    *(.text)
    }
    /* seção de dados */
    .data :
    {
    *(.data)
    }
}