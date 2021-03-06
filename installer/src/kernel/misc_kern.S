/*	Cafe OS Substrate Installer

	misc_kern.S - Miscellaneous kernel-related functions, such as kern_read/write.
	Paired with kernel.h.

	https://github.com/QuarkTheAwesome/COSSubstrate

	Copyright (c) 2016 Ash (QuarkTheAwesome)
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to
	deal in the Software without restriction, including without limitation the
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "ppc_asm.h"

.set KERN_SYSCALL_TBL_1,	0xFFE84C70 // unknown
.set KERN_SYSCALL_TBL_2,	0xFFE85070 // works with games (us, Mii Maker)
.set KERN_SYSCALL_TBL_3,	0xFFE85470 // works with loader
.set KERN_SYSCALL_TBL_4,	0xFFEAAA60 // works with home menu
.set KERN_SYSCALL_TBL_5,	0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

.set IMPROVED_KERN_READ,	0xFFF02044 //lwzx r3, r9, r10; rfi
.set IMPROVED_KERN_WRITE, 	0xFFF02068 //stwx r5, r9, r10; rfi

/*	Installs some of the various kernelmode functions used by the installer and the Substrate.
	void InstallKernFunctions()
*/
.globl InstallKernFunctions
InstallKernFunctions:
	mflr r0
	stw r0, 8(r1) //TODO - is this 4 or 8? Does it matter?
	stwu r1, -16(r1)

	//Inject syscall 0x2300 (improved_kern_read)
	lis r3, (KERN_SYSCALL_TBL_2 + 0x8C)@ha //0x8C = 0x23 * 4
	ori r3, r3, (KERN_SYSCALL_TBL_2 + 0x8C)@l

	lis r4, IMPROVED_KERN_READ@ha
	ori r4, r4, IMPROVED_KERN_READ@l

	bl old_kern_write

	//Inject syscall 0x2400 (improved_kern_write)
	lis r3, (KERN_SYSCALL_TBL_2 + 0x90)@ha //0x90 = 0x24 * 4
	ori r3, r3, (KERN_SYSCALL_TBL_2 + 0x90)@l

	lis r4, IMPROVED_KERN_WRITE@ha
	ori r4, r4, IMPROVED_KERN_WRITE@l

	bl old_kern_write

	//Put everything back to normal and return
	addi r1, r1, 16
	lwz r0, 8(r1)
	mtlr r0

	blr

/*	All kern functions need somewhere to keep the stack pointer.
	This is it.
*/
stack_pointer_save:
	.int 0x00000000

/*	My take on an imporoved kern_read.
	I tried to make it superscalar where I could, which makes it tough to read.
	Sorry.
	unsigned int kern_read(const void* addr)
*/
.globl kern_read
kern_read:
	mflr r0 //C calling convention
	mr r9, r3 //Move addr into r9
	stw r0, 8(r1)
	lis r4, stack_pointer_save@ha
	li r10, 0
	stw r1, stack_pointer_save@l(r4)

	//Syscall 0x2300 points to lwzx r3, r9, r10; rfi
	li r0, 0x2300
	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	lwz r0, 8(r1)
	mtlr r0

	blr

/*	My take on an imporoved kern_write.
	void kern_write(const void* addr, unsigned int val)
*/
.globl kern_write
kern_write:
	mflr r0 //C calling convention
	mr r9, r3 //Move addr into r9
	stw r0, 8(r1)
	mr r5, r4
	lis r4, stack_pointer_save@ha
	li r10, 0
	stw r1, stack_pointer_save@l(r4)

	//Syscall 0x2400 points to stwx r5, r9, r10; rfi
	li r0, 0x2400
	sc
	nop

	lis r4, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r4)

	lwz r0, 8(r1)
	mtlr r0

	blr

/*	Unoptimised, horrific kern_write. The syscall itself isn't much good either.
	void old_kern_write(const void* addr, unsigned int val)
*/
.globl old_kern_write
old_kern_write:
	mflr r0
	stw r0, 8(r1)

	mr r5, r4
	mr r9, r3

	li r3, 1
	li r4, 0
	li r6, 0
	li r7, 0
	lis r8, 1

	lis r10, stack_pointer_save@ha
	stw r1, stack_pointer_save@l(r10)

	li r0, 0x3500
	sc
	nop

	lis r10, stack_pointer_save@ha
	lwz r1, stack_pointer_save@l(r10)

	lwz r0, 8(r1)
	mtlr r0

	blr


.globl ALongRoutine
ALongRoutine:
	mr r9, r3
	nop
	mr r10, r4
	nop
	add r5, r9, r10
	addi r5, r5, 0x7ABC
	mr r3, r5
	blr
