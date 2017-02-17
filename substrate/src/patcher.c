/*	Cafe OS Substrate

	patcher.c - Arranging, generating and patching PowerPC code
	Paired with patches.h.

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

#include <stdlib.h>
#include <string.h>
#include "dynamic_libs/mem_functions.h"
#include "dynamic_libs/os_functions.h"
#include "kernel/kernel.h"
#include "patches/patches.h"
#include "utils/hash.h"

#include <substrate/substrate.h>

//But first... A hash table.
//TODO Mutexes on the hash table? Maybe?

typedef struct _COSSubstrate_PatchedFunction {
	/* Make sure to keep this aligned to 0x4! */
	unsigned int mtctr_r2;
	unsigned char origInstructions[MAIN_PATCH_SIZE];
	unsigned int bctr;

	int addr;

	void* next;
} COSSubstrate_PatchedFunction;

COSSubstrate_PatchedFunction** patchedFunctions = 0;

void private_addToFunctionHashTable(COSSubstrate_PatchedFunction* patch) {
	unsigned short hash = hashInt((unsigned int)(patch->addr));
	if (patchedFunctions) {
		if (patchedFunctions[hash]) {
			//hash collision!
			COSSubstrate_PatchedFunction* p = patchedFunctions[hash];
			while (p != 0) {
				p = (COSSubstrate_PatchedFunction*)p->next;
			}
			p->next = (void*)patch;
		} else {
			patchedFunctions[hash] = patch;
		}
	} else {
		patchedFunctions = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, 0xFFFF * sizeof(COSSubstrate_PatchedFunction*), 0x4); //oh man
		if (!patchedFunctions) OSFatal("Couldn't allocate patched function hash table!");
		patchedFunctions[hash] = patch;
	}
}

COSSubstrate_PatchedFunction* private_lookupFromFunctionHashTable(unsigned int addr) {
	if (!patchedFunctions) return 0;

	unsigned short hash = hashInt(addr);
	if (patchedFunctions[hash]) {
		if (patchedFunctions[hash]->addr == addr) return patchedFunctions[hash];
		COSSubstrate_PatchedFunction* p = patchedFunctions[hash];
		while (p != 0) {
			p = (COSSubstrate_PatchedFunction*)p->next;
		}
		return p;
	} else return 0;
}

//And now... your function patcher

/*	WIP function to see if my codegen works okay
	From this alone you can probably tell this is going to be a very
	different type of patcher to what we're used to

	I think "function patcher" and I think actually overwriting the function :3
*/
void COSSubstrate_PatchFunc(void* func) {
	COSSubstrate_PatchedFunction* patch = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, sizeof(COSSubstrate_PatchedFunction), 0x4);
	patch->addr = (unsigned int)func;

	//Some codegen for 'yall
	memcpy(patch->origInstructions, func, MAIN_PATCH_SIZE);
	patch->bctr = *(unsigned int*)(&bctr);
	patch->mtctr_r2 = *(unsigned int*)(&mtctr_r2);

	private_addToFunctionHashTable(patch);

	DCFlushRange(patch, sizeof(COSSubstrate_PatchedFunction));
	ICInvalidateRange(patch, sizeof(COSSubstrate_PatchedFunction));

	unsigned int* t = (unsigned int*)&MAIN_PATCH;
	//TODO this is ugly but looping over MAIN_PATCH_SIZE bugs out so often that I don't care
	kern_write(func, t[0]);
	kern_write(func + 4, t[1]);
	kern_write(func + 8, t[2]);
	kern_write(func + 0xC, t[3]);
	kern_write(func + 0x10, t[4]);
}

void COSSubstrate_RestoreFunc(void* func) {
	COSSubstrate_PatchedFunction* patch = private_lookupFromFunctionHashTable((unsigned int)func);
	if (!patch) return;

	unsigned int* t = (unsigned int*)patch->origInstructions;

	kern_write(func, t[0]);
	kern_write(func + 4, t[1]);
	kern_write(func + 8, t[2]);
	kern_write(func + 0xC, t[3]);
	kern_write(func + 0x10, t[4]);

	//TODO remove patch from hash table and free memory
}

/*	Called by the Assembly to handle making the struct.
	Sure, I could do it with pure ASM, but I'm too lazy for all those #defines.
*/
COSSubstrate_FunctionContext* private_generateFunctionContext(unsigned int* args_in, unsigned int lr) {
	COSSubstrate_FunctionContext* ctx = MEMAllocFromExpHeapEx(COSS_MAIN_HEAP, sizeof(COSSubstrate_FunctionContext), 0x4);

	ctx->source = (void*)(lr - MAIN_PATCH_SIZE);

	//Save this for the Assembly later on.
	ctx->substrate_internal = (unsigned int)args_in;
	//TODO this
	ctx->args[0] = args_in[0];
	ctx->args[1] = args_in[1];
	ctx->args[2] = args_in[2];
	ctx->args[3] = args_in[3];
	ctx->args[4] = args_in[4];

	return ctx;
}

void (*debug_callback)(COSSubstrate_FunctionContext* ctx);

/*	Called by the Assmebly to route out the C callbacks.
	Also returns the first of the original instructions to make life easier
	on the ASM side of things.
*/
void* private_dispatchCallbacksAndGetInstrunctions(COSSubstrate_FunctionContext* ctx) {
	debug_callback(ctx);

	/*	hey! hey! ctx->substrate_internal is controlled by the modules!
		that's terrible sandboxing!
		Guess what - Modules can take full control of the kernel if they want.
		There's really no point trying to sandbox them, and this has the benefit
		of being thread-safe.
	*/
	unsigned int* args_out = (unsigned int*)(ctx->substrate_internal);
	//TODO this
	args_out[0] = ctx->args[0];
	args_out[1] = ctx->args[1];
	args_out[2] = ctx->args[2];
	args_out[3] = ctx->args[3];
	args_out[4] = ctx->args[4];

	unsigned int source = (unsigned int)ctx->source;

	MEMFreeToExpHeap(COSS_MAIN_HEAP, ctx);

	return (void*)&((private_lookupFromFunctionHashTable(source))->mtctr_r2);
}

/*	I haven't gotten around to setting up a proper way of keeping track of callbacks.
	TODO, I guess.
*/
void debug_setCallback(void(*func)(COSSubstrate_FunctionContext* ctx)) {
	debug_callback = func;
}
