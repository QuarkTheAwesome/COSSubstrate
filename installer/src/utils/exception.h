/*	Cafe OS Substrate Installer

	exception.h - Exception handler.
	No parter file.

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

/* Ash's Infamous Exception Handler! */
/* Failing to deal with multithreaded apps since 2016. */

#include "dynamic_libs/os_functions.h"

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

void exception_disassembly_helper(char *fmt, int addr,int opcode, char* s)
{
    char* *store = (char**)0x1ab5d140;
    char *buffer = (char *)store[0];
    if (addr == ((int*)store)[1]) {
    	store[0] += __os_snprintf(buffer,512,"> 0x%08X    0x%08X    %s\n", addr,opcode,s);
    } else {
    	store[0] += __os_snprintf(buffer,512,"  0x%08X    0x%08X    %s\n", addr,opcode,s);
    }

}
unsigned char exception_handler(void* contextIn, int type) {
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	void (*DisassemblePPCRange)(void *start, void *end, void *printf_func, int *find_symbol_func, int flags);
	OSDynLoad_FindExport(coreinit_handle, 0, "DisassemblePPCRange", &DisassemblePPCRange);

	int* context = (int*)contextIn;

	char buf2[512];
	int* store = (int*)0x1AB5D140;
	store[0] = (int)buf2;
	store[1] = (int)context[38];


	DisassemblePPCRange((void*)context[38]-0x18, (void*)context[38]+0x4, (void*)exception_disassembly_helper, 0, 0);

	char crashMessage[128];
	__os_snprintf(crashMessage, 128, "Exception type %d occured. Aww. Disasm at PC.", type);

	char buf[2048];
	__os_snprintf(buf, 2048, "%s\nSP:%08X  LR:%08X  PC:%08X  CR:%08X  CTR:%08X\nXER:%08X SR0:%08X SR1:%08X EX0:%08X EX1:%08X\nr0:%08X  r2:%08X  r3:%08X  r4:%08X  r5:%08X\nr6:%08X  r7:%08X  r8:%08X  r9:%08X  r10:%08X\nr11:%08X r12:%08X r13:%08X r14:%08X r15:%08X\nr16:%08X r17:%08X r18:%08X r19:%08X r20:%08X\nr21:%08X r22:%08X r23:%08X r24:%08X r25:%08X\nr26:%08X r27:%08X r28:%08X r29:%08X r30:%08X\nr31:%08X\n%s", crashMessage, context[3], context[35], context[38], context[34], context[36], context[37], context[38], context[39], context[40], context[41], context[2], context[4], context[5], context[6], context[7], context[8], context[9], context[10], context[11], context[12], context[13], context[14], context[15], context[16], context[17], context[18], context[19], context[20], context[21], context[22], context[23], context[24], context[25], context[26], context[27], context[28], context[29], context[30], context[31], context[32], context[33], buf2);
	OSFatal(buf);

	return 0;
}
unsigned char exception_handler_dsi(void* contextIn) {
	return exception_handler(contextIn, 2);
}
unsigned char exception_handler_isi(void* contextIn) {
	return exception_handler(contextIn, 3);
}
unsigned char exception_handler_pro(void* contextIn) {
	return exception_handler(contextIn, 6);
}
void InstallExceptionHandler() {
	OSSetExceptionCallback(2, &exception_handler_dsi);
	OSSetExceptionCallback(3, &exception_handler_isi);
	OSSetExceptionCallback(6, &exception_handler_pro);
}

unsigned char adv_pc(void* ctx) {
	((unsigned int*)ctx)[38] += 4;
	((unsigned int*)ctx)[6] = 0xDEADC0DE;

	return 1;
}
void InstallAltExceptionHandler() {
	OSSetExceptionCallback(2, &adv_pc);
	OSSetExceptionCallback(3, &adv_pc);
	OSSetExceptionCallback(6, &adv_pc);
}
#endif //__EXCEPTION_H__
