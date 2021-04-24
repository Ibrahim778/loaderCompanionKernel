.arch armv7a

.section .vitalink.fstubs.KernelLaunchFile,"ax",%progbits

	.align 4
	.global launchAppFromFileExport
	.type launchAppFromFileExport, %function
launchAppFromFileExport:
.if GEN_WEAK_EXPORTS
	.word 0x00010008
.else
	.word 0x00010000
.endif //GEN_WEAK_EXPORTS
	.word 0x2ED46EF6
	.word 0x801F0B1D
	.align 4

