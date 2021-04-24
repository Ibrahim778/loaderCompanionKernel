.arch armv7a

.section .vitalink.fstubs.LoaderCompanionKernel,"ax",%progbits

	.align 4
	.global StopUsbExp
	.type StopUsbExp, %function
StopUsbExp:
.if GEN_WEAK_EXPORTS
	.word 0x00010008
.else
	.word 0x00010000
.endif //GEN_WEAK_EXPORTS
	.word 0x77DE860F
	.word 0xC9F74145
	.align 4

