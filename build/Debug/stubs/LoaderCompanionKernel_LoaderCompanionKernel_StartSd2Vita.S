.arch armv7a

.section .vitalink.fstubs.LoaderCompanionKernel,"ax",%progbits

	.align 4
	.global StartSd2Vita
	.type StartSd2Vita, %function
StartSd2Vita:
.if GEN_WEAK_EXPORTS
	.word 0x00010008
.else
	.word 0x00010000
.endif //GEN_WEAK_EXPORTS
	.word 0x77DE860F
	.word 0x1C6048CC
	.align 4

