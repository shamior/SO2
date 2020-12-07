.option norvc

.section .text
.global mvector
# alinhamento de 4 porque os últimos 2 bits do 
# registrador mtvec não contribuem para o endereço do vetor
.balign 4
mvector:
	# Salva todos os registradores na região de memória apontada 
	# por mscratch (trap frame - TF). Escolhemos t6 para guardar o endereço
	# do trap frame. 
	# A instrução usa dois registradores inteiros. Um é p/ copiar 
	# o reg. csr (destino) e o outro é de onde o novo valor é copiado
	# (origem) para o csr.	 Formato:
	# CSRRW rd,csr, rs1 (rd = destino)
	csrrw	t6, mscratch, t6
	# csrrw troca os valores de t6 e mscratch, 
	# atomicamente, o que mantém os registradors intactos.

	sd ra, 0(t6)
	sd sp, 8(t6)
	sd gp, 16(t6)
	sd tp, 24(t6)
	sd t0, 32(t6)
	sd t1, 40(t6)
	sd t2, 48(t6)
	sd s0, 56(t6)
	sd s1, 64(t6)
	sd a0, 72(t6)
	sd a1, 80(t6)
	sd a2, 88(t6)
	sd a3, 96(t6)
	sd a4, 104(t6)
	sd a5, 112(t6)
	sd a6, 120(t6)
	sd a7, 128(t6)
	sd s2, 136(t6)
	sd s3, 144(t6)
	sd s4, 152(t6)
	sd s5, 160(t6)
	sd s6, 168(t6)
	sd s7, 176(t6)
	sd s8, 184(t6)
	sd s9, 192(t6)
	sd s10, 200(t6)
	sd s11, 208(t6)
	sd t3, 216(t6)
	sd t4, 224(t6)
	sd t5, 232(t6)

	# Salva t6 (no TF), cujo valor está temporariamente
	# em mscratch. t5 passa a apontar o TF.
	# mv rd, rs pseudo-instrução para addi rd, rs, 0
	mv		t5, t6
	#pseudo-instrução para csrrs rd, csr, x0 (csr read and set)
	# csrr	t6, mscratch
	
	# restaura t6 e mscratch
	csrrw t6, mscratch, t6
	sd t6, 240(t5)

	# Restaura o end. de TF no mscratch
	# pseudo-instrução para csrrw x0, csr, rs
	# csrw	mscratch, t5

	# copia epc e hartid para o TF
	csrr	t0, mepc
	csrr	t1, mhartid
	sd      t1, 272(t5) # tf.hartid
	sd      t0, 280(t5) # tf.epc


	# Antes de ir para o código C (trap.c)
	# sp passa a apontar a pilha do kernel reservada
	# para lidar com os traps (interrupções, exceções, syscall)
	# A função m_trap recebe 4 argumentos:
	csrr	a0, mtval
	csrr	a1, mcause
	csrr	a2, mstatus
	mv		a3, t5 # a3 aponta TF
	ld		sp, 264(a3)
	call	mtrap

	# Neste ponto retornamos da função em C mtrap. É preciso restaurar os 
	# registradores antes de voltar ao modo menos privilegiado
	
	# Carrega o TF em t6
	csrr	t6, mscratch
	# Obtém endereço de retorno do TF.
	ld t5, 280(t6)  #TF.epc
	# csrw: A origem precisa ser um registrador  (e não um imediato),
	# por isso o uso de um registrador temporário (no caso, o t5)
	csrw mepc, t5

	# Restaura os registradores 
	ld ra, 0(t6)
	ld sp, 8(t6)  # restaura a pilha do código pré-exceção
	ld gp, 16(t6)
	ld tp, 24(t6)
	ld t0, 32(t6)
	ld t1, 40(t6)
	ld t2, 48(t6)
	ld s0, 56(t6)
	ld s1, 64(t6)
	ld a0, 72(t6)
	ld a1, 80(t6)
	ld a2, 88(t6)
	ld a3, 96(t6)
	ld a4, 104(t6)
	ld a5, 112(t6)
	ld a6, 120(t6)
	ld a7, 128(t6)
	ld s2, 136(t6)
	ld s3, 144(t6)
	ld s4, 152(t6)
	ld s5, 160(t6)
	ld s6, 168(t6)
	ld s7, 176(t6)
	ld s8, 184(t6)
	ld s9, 192(t6)
	ld s10, 200(t6)
	ld s11, 208(t6)
	ld t3, 216(t6)
	ld t4, 224(t6)
	ld t5, 232(t6)
	ld t6, 240(t6)

	# Por último, t6, que apontava para TF, também
	# foi restaurado.

	mret