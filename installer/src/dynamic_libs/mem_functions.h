/*	Cafe OS Substrate Installer

	mem_functions.h - Exposes memory functions from dynamic_libs.
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


extern void *(* MEMAllocFromExpHeapEx)(int heap, unsigned int size, int align);
extern int (* MEMCreateExpHeapEx)(void* address, unsigned int size, unsigned short flags);
extern void *(* MEMDestroyExpHeap)(int heap);
extern void (* MEMFreeToExpHeap)(int heap, void* ptr);

extern unsigned int* pMEMAllocFromDefaultHeapEx;
extern unsigned int* pMEMFreeToDefaultHeap;

#define MEMAllocFromDefaultHeapEx(x, y) ((void * (*)(unsigned int, int))(*pMEMAllocFromDefaultHeapEx))(x, y);
#define MEMFreeToDefaultHeap(x) ((void (*)(void *))(*pMEMFreeToDefaultHeap))(x);
