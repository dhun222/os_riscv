
kernel/kernel:     file format elf64-littleriscv


Disassembly of section .text:

0000000080000000 <_boot>:
    80000000:	00001117          	auipc	sp,0x1
    80000004:	01010113          	addi	sp,sp,16 # 80001010 <stack_start>
    80000008:	f1402573          	csrr	a0,mhartid
    8000000c:	6585                	lui	a1,0x1
    8000000e:	02a585b3          	mul	a1,a1,a0
    80000012:	912e                	add	sp,sp,a1
    80000014:	004000ef          	jal	80000018 <init>

0000000080000018 <init>:
    80000018:	1141                	addi	sp,sp,-16
    8000001a:	e406                	sd	ra,8(sp)
    8000001c:	e022                	sd	s0,0(sp)
    8000001e:	0800                	addi	s0,sp,16
    80000020:	4781                	li	a5,0
    80000022:	18079073          	csrw	satp,a5
    80000026:	300027f3          	csrr	a5,mstatus
    8000002a:	7779                	lui	a4,0xffffe
    8000002c:	7ff70713          	addi	a4,a4,2047 # ffffffffffffe7ff <uart_lock+0xffffffff7fffb7ef>
    80000030:	8ff9                	and	a5,a5,a4
    80000032:	6705                	lui	a4,0x1
    80000034:	80870713          	addi	a4,a4,-2040 # 808 <_boot-0x7ffff7f8>
    80000038:	8fd9                	or	a5,a5,a4
    8000003a:	30079073          	csrw	mstatus,a5
    8000003e:	67c1                	lui	a5,0x10
    80000040:	37fd                	addiw	a5,a5,-1 # ffff <_boot-0x7fff0001>
    80000042:	30379073          	csrw	mideleg,a5
    80000046:	30279073          	csrw	medeleg,a5
    8000004a:	47bd                	li	a5,15
    8000004c:	3a079073          	csrw	pmpcfg0,a5
    80000050:	57fd                	li	a5,-1
    80000052:	83a1                	srli	a5,a5,0x8
    80000054:	3b079073          	csrw	pmpaddr0,a5
    80000058:	00000797          	auipc	a5,0x0
    8000005c:	02a78793          	addi	a5,a5,42 # 80000082 <main>
    80000060:	34179073          	csrw	mepc,a5
    80000064:	822a                	mv	tp,a0
    80000066:	30200073          	mret
    8000006a:	60a2                	ld	ra,8(sp)
    8000006c:	6402                	ld	s0,0(sp)
    8000006e:	0141                	addi	sp,sp,16
    80000070:	8082                	ret

0000000080000072 <enableIntr>:
    80000072:	1141                	addi	sp,sp,-16
    80000074:	e406                	sd	ra,8(sp)
    80000076:	e022                	sd	s0,0(sp)
    80000078:	0800                	addi	s0,sp,16
    8000007a:	60a2                	ld	ra,8(sp)
    8000007c:	6402                	ld	s0,0(sp)
    8000007e:	0141                	addi	sp,sp,16
    80000080:	8082                	ret

0000000080000082 <main>:
    80000082:	1141                	addi	sp,sp,-16
    80000084:	e406                	sd	ra,8(sp)
    80000086:	e022                	sd	s0,0(sp)
    80000088:	0800                	addi	s0,sp,16
    8000008a:	072000ef          	jal	800000fc <initUART>
    8000008e:	0e8000ef          	jal	80000176 <initPLIC>
    80000092:	0f4000ef          	jal	80000186 <initDisk>
    80000096:	100000ef          	jal	80000196 <sched>
    8000009a:	60a2                	ld	ra,8(sp)
    8000009c:	6402                	ld	s0,0(sp)
    8000009e:	0141                	addi	sp,sp,16
    800000a0:	8082                	ret

00000000800000a2 <initSpinLock>:
    800000a2:	1141                	addi	sp,sp,-16
    800000a4:	e406                	sd	ra,8(sp)
    800000a6:	e022                	sd	s0,0(sp)
    800000a8:	0800                	addi	s0,sp,16
    800000aa:	0511                	addi	a0,a0,4
    800000ac:	0aa000ef          	jal	80000156 <strcpy>
    800000b0:	60a2                	ld	ra,8(sp)
    800000b2:	6402                	ld	s0,0(sp)
    800000b4:	0141                	addi	sp,sp,16
    800000b6:	8082                	ret

00000000800000b8 <acquireSpinLock>:
    800000b8:	1141                	addi	sp,sp,-16
    800000ba:	e406                	sd	ra,8(sp)
    800000bc:	e022                	sd	s0,0(sp)
    800000be:	0800                	addi	s0,sp,16
    800000c0:	4110                	lw	a2,0(a0)
    800000c2:	4685                	li	a3,1
    800000c4:	4705                	li	a4,1
    800000c6:	08d627af          	amoswap.w	a5,a3,(a2)
    800000ca:	2781                	sext.w	a5,a5
    800000cc:	fee78de3          	beq	a5,a4,800000c6 <acquireSpinLock+0xe>
    800000d0:	60a2                	ld	ra,8(sp)
    800000d2:	6402                	ld	s0,0(sp)
    800000d4:	0141                	addi	sp,sp,16
    800000d6:	8082                	ret

00000000800000d8 <releaseSpinLock>:
    800000d8:	1141                	addi	sp,sp,-16
    800000da:	e406                	sd	ra,8(sp)
    800000dc:	e022                	sd	s0,0(sp)
    800000de:	0800                	addi	s0,sp,16
    800000e0:	00052023          	sw	zero,0(a0)
    800000e4:	60a2                	ld	ra,8(sp)
    800000e6:	6402                	ld	s0,0(sp)
    800000e8:	0141                	addi	sp,sp,16
    800000ea:	8082                	ret

00000000800000ec <destSpinLock>:
    800000ec:	1141                	addi	sp,sp,-16
    800000ee:	e406                	sd	ra,8(sp)
    800000f0:	e022                	sd	s0,0(sp)
    800000f2:	0800                	addi	s0,sp,16
    800000f4:	60a2                	ld	ra,8(sp)
    800000f6:	6402                	ld	s0,0(sp)
    800000f8:	0141                	addi	sp,sp,16
    800000fa:	8082                	ret

00000000800000fc <initUART>:
    800000fc:	1141                	addi	sp,sp,-16
    800000fe:	e406                	sd	ra,8(sp)
    80000100:	e022                	sd	s0,0(sp)
    80000102:	0800                	addi	s0,sp,16
    80000104:	100007b7          	lui	a5,0x10000
    80000108:	000780a3          	sb	zero,1(a5) # 10000001 <_boot-0x6fffffff>
    8000010c:	10000737          	lui	a4,0x10000
    80000110:	f8000693          	li	a3,-128
    80000114:	00d708a3          	sb	a3,17(a4) # 10000011 <_boot-0x6fffffef>
    80000118:	468d                	li	a3,3
    8000011a:	10000637          	lui	a2,0x10000
    8000011e:	00d60023          	sb	a3,0(a2) # 10000000 <_boot-0x70000000>
    80000122:	000780a3          	sb	zero,1(a5)
    80000126:	000708a3          	sb	zero,17(a4)
    8000012a:	00d708a3          	sb	a3,17(a4)
    8000012e:	8732                	mv	a4,a2
    80000130:	461d                	li	a2,7
    80000132:	00c70823          	sb	a2,16(a4)
    80000136:	00d780a3          	sb	a3,1(a5)
    8000013a:	00001597          	auipc	a1,0x1
    8000013e:	ec658593          	addi	a1,a1,-314 # 80001000 <sched+0xe6a>
    80000142:	00003517          	auipc	a0,0x3
    80000146:	ece50513          	addi	a0,a0,-306 # 80003010 <uart_lock>
    8000014a:	f59ff0ef          	jal	800000a2 <initSpinLock>
    8000014e:	60a2                	ld	ra,8(sp)
    80000150:	6402                	ld	s0,0(sp)
    80000152:	0141                	addi	sp,sp,16
    80000154:	8082                	ret

0000000080000156 <strcpy>:
    80000156:	1141                	addi	sp,sp,-16
    80000158:	e406                	sd	ra,8(sp)
    8000015a:	e022                	sd	s0,0(sp)
    8000015c:	0800                	addi	s0,sp,16
    8000015e:	0005c783          	lbu	a5,0(a1)
    80000162:	c791                	beqz	a5,8000016e <strcpy+0x18>
    80000164:	00f50023          	sb	a5,0(a0)
    80000168:	0005c783          	lbu	a5,0(a1)
    8000016c:	ffe5                	bnez	a5,80000164 <strcpy+0xe>
    8000016e:	60a2                	ld	ra,8(sp)
    80000170:	6402                	ld	s0,0(sp)
    80000172:	0141                	addi	sp,sp,16
    80000174:	8082                	ret

0000000080000176 <initPLIC>:
    80000176:	1141                	addi	sp,sp,-16
    80000178:	e406                	sd	ra,8(sp)
    8000017a:	e022                	sd	s0,0(sp)
    8000017c:	0800                	addi	s0,sp,16
    8000017e:	60a2                	ld	ra,8(sp)
    80000180:	6402                	ld	s0,0(sp)
    80000182:	0141                	addi	sp,sp,16
    80000184:	8082                	ret

0000000080000186 <initDisk>:
    80000186:	1141                	addi	sp,sp,-16
    80000188:	e406                	sd	ra,8(sp)
    8000018a:	e022                	sd	s0,0(sp)
    8000018c:	0800                	addi	s0,sp,16
    8000018e:	60a2                	ld	ra,8(sp)
    80000190:	6402                	ld	s0,0(sp)
    80000192:	0141                	addi	sp,sp,16
    80000194:	8082                	ret

0000000080000196 <sched>:
    80000196:	1141                	addi	sp,sp,-16
    80000198:	e406                	sd	ra,8(sp)
    8000019a:	e022                	sd	s0,0(sp)
    8000019c:	0800                	addi	s0,sp,16
    8000019e:	60a2                	ld	ra,8(sp)
    800001a0:	6402                	ld	s0,0(sp)
    800001a2:	0141                	addi	sp,sp,16
    800001a4:	8082                	ret
	...
