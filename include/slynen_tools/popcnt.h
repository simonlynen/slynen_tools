/*
 * popcnt.h
 *
 *  Created on: 21.01.2011
 *      Author: slynen
 */

#ifndef POPCNT_H_
#define POPCNT_H_

#include <emmintrin.h>
#include <tmmintrin.h>
#include <stdint.h>

namespace popcnt{

// this is needed to avoid aliasing issues with the __m128i data type:
typedef unsigned char __attribute__ ((__may_alias__)) UCHAR_ALIAS;
typedef u_int8_t __attribute__ ((__may_alias__)) U_INT8T_ALIAS;
typedef unsigned short __attribute__ ((__may_alias__)) UINT16_ALIAS;
typedef unsigned int __attribute__ ((__may_alias__)) UINT32_ALIAS;
typedef int __attribute__ ((__may_alias__)) INT32_ALIAS;
typedef float __attribute__ ((__may_alias__)) FLOAT_ALIAS;

static const __m128i shiftval = _mm_set_epi32 (0,0,0,4);

//by http://bmagic.sourceforge.net/bmsse2opt.html
inline unsigned popcount_sse2(__m128i* block)
{
	const unsigned mu1 = 0x55555555;
	const unsigned mu2 = 0x33333333;
	const unsigned mu3 = 0x0F0F0F0F;
	const unsigned mu4 = 0x0000003F;

	// Loading masks
	__m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
	__m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
	__m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
	__m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);

	__m128i tmp1, tmp2;
	__m128i b = _mm_load_si128(block);

	// b = (b & 0x55555555) + (b >> 1 & 0x55555555);
	tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
	tmp1 = _mm_and_si128(tmp1, m1);
	tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
	b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

	// b = (b & 0x33333333) + (b >> 2 & 0x33333333);
	tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
	tmp1 = _mm_and_si128(tmp1, m2);
	tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
	b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

	// b = (b + (b >> 4)) & 0x0F0F0F0F;
	tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
	b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
	b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

	// b = b + (b >> 8);
	tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
	b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)

	// b = (b + (b >> 16)) & 0x0000003F;
	tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
	b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
	b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

	const UINT32_ALIAS* ptr=reinterpret_cast<const UINT32_ALIAS*>(&b);
	return ptr[0] + ptr[1] + ptr[2] + ptr[3];
}

inline int popcount_sse2(const __m128i* data,
		const int numberOf128BitWords)
{
	const unsigned mu1 = 0x55555555;
	const unsigned mu2 = 0x33333333;
	const unsigned mu3 = 0x0F0F0F0F;
	const unsigned mu4 = 0x0000003F;

	// Loading masks
	__m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
	__m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
	__m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
	__m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);
	__m128i mcnt;
	//mcnt = _mm_xor_si128(mcnt, mcnt); // cnt = 0
	mcnt = _mm_setzero_si128 ();


	for(int i = 0;i<numberOf128BitWords;i++)
	{
		__m128i tmp1, tmp2;
		__m128i b = *(data+i);

		// b = (b & 0x55555555) + (b >> 1 & 0x55555555);
		tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
		tmp1 = _mm_and_si128(tmp1, m1);
		tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
		b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

		// b = (b & 0x33333333) + (b >> 2 & 0x33333333);
		tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
		tmp1 = _mm_and_si128(tmp1, m2);
		tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
		b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

		// b = (b + (b >> 4)) & 0x0F0F0F0F;
		tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
		b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

		// b = b + (b >> 8);
		tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)

		// b = (b + (b >> 16)) & 0x0000003F;
		tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
		b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
		b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

		mcnt = _mm_add_epi32(mcnt, b);                  // mcnt += b

	}
	const UINT32_ALIAS* ptr=reinterpret_cast<const UINT32_ALIAS*>(&mcnt);
	return ptr[0] + ptr[1] + ptr[2] + ptr[3];
}

inline int popcntOfXored(const __m128i* data1, const __m128i* data2,
		const int numberOf128BitWords)
{
	const unsigned mu1 = 0x55555555;
	const unsigned mu2 = 0x33333333;
	const unsigned mu3 = 0x0F0F0F0F;
	const unsigned mu4 = 0x0000003F;

	// Loading masks
	__m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
	__m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
	__m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
	__m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);
	__m128i mcnt;
	//mcnt = _mm_xor_si128(mcnt, mcnt); // cnt = 0
	mcnt = _mm_setzero_si128 ();


	for(int i = 0;i<numberOf128BitWords;i++)
	{
		__m128i tmp1, tmp2;
		__m128i b = _mm_xor_si128 ( *(data1+i), *(data2+i));

		// b = (b & 0x55555555) + (b >> 1 & 0x55555555);
		tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
		tmp1 = _mm_and_si128(tmp1, m1);
		tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
		b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

		// b = (b & 0x33333333) + (b >> 2 & 0x33333333);
		tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
		tmp1 = _mm_and_si128(tmp1, m2);
		tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
		b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

		// b = (b + (b >> 4)) & 0x0F0F0F0F;
		tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
		b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

		// b = b + (b >> 8);
		tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)

		// b = (b + (b >> 16)) & 0x0000003F;
		tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
		b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
		b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

		mcnt = _mm_add_epi32(mcnt, b);                  // mcnt += b

	}
	const UINT32_ALIAS* ptr=reinterpret_cast<const UINT32_ALIAS*>(&mcnt);
	return ptr[0] + ptr[1] + ptr[2] + ptr[3];
}


//SSE2 overloaded to avoid additional code in the above function resulting from the "sum > max" branching
inline int popcntOfXored(const __m128i* data1, const __m128i* data2,
		const int numberOf128BitWords, int max)
{
	const unsigned mu1 = 0x55555555;
	const unsigned mu2 = 0x33333333;
	const unsigned mu3 = 0x0F0F0F0F;
	const unsigned mu4 = 0x0000003F;

	// Loading masks
	__m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
	__m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
	__m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
	__m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);
	__m128i mcnt = _mm_setzero_si128 ();
	//__m128i mcnt = _mm_xor_si128(mcnt, mcnt); // cnt = 0


	//	int ret[4];
	int sum =0;
	for(int i = 0;i<numberOf128BitWords;i++)
	{
		__m128i tmp1, tmp2;
		__m128i b = _mm_xor_si128 ( *(data1+i), *(data2+i));

		// b = (b & 0x55555555) + (b >> 1 & 0x55555555);
		tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
		tmp1 = _mm_and_si128(tmp1, m1);
		tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
		b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

		// b = (b & 0x33333333) + (b >> 2 & 0x33333333);
		tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
		tmp1 = _mm_and_si128(tmp1, m2);
		tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
		b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

		// b = (b + (b >> 4)) & 0x0F0F0F0F;
		tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
		b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

		// b = b + (b >> 8);
		tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
		b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)

		// b = (b + (b >> 16)) & 0x0000003F;
		tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
		b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
		b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

		mcnt = _mm_add_epi32(mcnt, b);                  // mcnt += b

		const UINT32_ALIAS* ptr=reinterpret_cast<const UINT32_ALIAS*>(&mcnt);
		sum = ptr[0] + ptr[1] + ptr[2] + ptr[3];
		if(sum>max) //premature cancelling
			break;
	}
	return sum;
}

static const char __attribute__((aligned(16))) MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
static const uint8_t __attribute__((aligned(16))) POPCOUNT_4bit[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

inline // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcount(__m128i* signature, int numberOf128BitWords) {

	uint32_t result;

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));
	__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // xmm5 -- global accumulator

	result = 0;

	__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
	const size_t end=(size_t)(signature+numberOf128BitWords);

	while((size_t)signature<end){ // lestefan - small reduction of overhead
		__asm__ volatile(
				"movdqa	  (%%eax), %%xmm0	\n"
				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $4, %%xmm1	\n"
				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
				"movdqa    %%xmm7, %%xmm2	\n"
				"movdqa    %%xmm7, %%xmm3	\n"	// get popcount
				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
				"paddb     %%xmm2, %%xmm4	\n"	// update local
				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
				:
				: "a" (signature++)
		);
	}
	// update global accumulator (two 32-bits counters)
	__asm__ volatile (
			"pxor	%xmm0, %xmm0		\n"
			"psadbw	%xmm0, %xmm4		\n"
			"paddd	%xmm4, %xmm5		\n"
	);
	// finally add together 32-bits counters stored in global accumulator
	__asm__ volatile (
			"movhlps   %%xmm5, %%xmm0	\n"
			"paddd     %%xmm5, %%xmm0	\n"
			"movd      %%xmm0, %%eax	\n"
			: "=a" (result)
	);
	return result;
}

__inline__ // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcount_intel(const __m128i* signature, const int numberOf128BitWords) {
	//static const char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
	//static const uint8_t POPCOUNT_4bit[16] __attribute__((aligned(16))) = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

	const U_INT8T_ALIAS* buffer = (U_INT8T_ALIAS*)(signature);
	uint32_t result = 0;

	register __m128i xmm0;
	register __m128i xmm1;
	register __m128i xmm2;
	register __m128i xmm3;
	register __m128i xmm4;
	register __m128i xmm5;
	register __m128i xmm6;
	register __m128i xmm7;

	//__asm__ volatile ("movdqa (%0), %%xmm7" : : "a" (POPCOUNT_4bit) : "xmm7");
	xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
	//__asm__ volatile ("movdqa (%0), %%xmm6" : : "a" (MASK_4bit) : "xmm6");
	xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
	//__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : : "xmm5"); // xmm5 -- global accumulator
	xmm5 = _mm_setzero_si128();

	const size_t end=(size_t)(buffer+numberOf128BitWords);

	//__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
	xmm4 = xmm5;//_mm_load_si128(&xmm5);


	//for (n=0; n < numberOf128BitWords; n++) {
	while((size_t)buffer<end){
		//		__asm__ volatile(
		//				"movdqa	  (%0), %%xmm0	\n"
		xmm0 = _mm_load_si128((__m128i*)buffer++);
		//				"movdqu    %%xmm0, %%xmm1	\n"
		xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
		//				"psrlw         $4, %%xmm1	\n"
		xmm1 = _mm_srl_epi16 (xmm1, shiftval);
		//				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
		xmm0 = _mm_and_si128 (xmm0, xmm6);
		//				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
		xmm1 = _mm_and_si128 (xmm1, xmm6);
		//				"movdqu    %%xmm7, %%xmm2	\n"
		xmm2 = xmm7;//_mm_loadu_si128(&xmm7);
		//				"movdqu    %%xmm7, %%xmm3	\n"	// get popcount
		xmm3 = xmm7;//_mm_loadu_si128(&xmm7);
		//				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
		xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
		//				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
		xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
		//				"paddb     %%xmm2, %%xmm4	\n"	// update local
		xmm4 = _mm_add_epi8(xmm4, xmm2);
		//				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
		xmm4 = _mm_add_epi8(xmm4, xmm3);
		//				:
		//				: "a" (buffer++)
		//				: "xmm0","xmm1","xmm2","xmm3","xmm4"
		//		);
	}
	// update global accumulator (two 32-bits counters)
	//	__asm__ volatile (
	//			/*"pxor	%xmm0, %xmm0		\n"*/
	//			"psadbw	%%xmm5, %%xmm4		\n"
	xmm4 = _mm_sad_epu8(xmm4, xmm5);
	//			"paddd	%%xmm4, %%xmm5		\n"
	xmm5 = _mm_add_epi32(xmm5, xmm4);
	//			:
	//			:
	//			: "xmm4","xmm5"
	//	);
	// finally add together 32-bits counters stored in global accumulator
//	__asm__ volatile (
//			"movhlps   %%xmm5, %%xmm0	\n"
	xmm0 = (__m128i)_mm_movehl_ps((__m128)xmm0, (__m128)xmm5); //TODO fix with appropriate intrinsic
//			"paddd     %%xmm5, %%xmm0	\n"
	xmm0 = _mm_add_epi32(xmm0, xmm5);
//			"movd      %%xmm0, %%eax	\n"
	result = _mm_cvtsi128_si32 (xmm0);
//			: "=a" (result) : : "xmm5","xmm0"
//	);
	return result;
}

inline // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcntofXORed(const __m128i* signature1, const __m128i* signature2, const __m128i* mask, const int numberOf128BitWords) {

	uint32_t result = 0;

		register __m128i xmm0;
		register __m128i xmm1;
		register __m128i xmm2;
		register __m128i xmm3;
		register __m128i xmm4;
		register __m128i xmm5;
		register __m128i xmm6;
		register __m128i xmm7;

		//__asm__ volatile ("movdqa (%0), %%xmm7" : : "a" (POPCOUNT_4bit) : "xmm7");
		xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
		//__asm__ volatile ("movdqa (%0), %%xmm6" : : "a" (MASK_4bit) : "xmm6");
		xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
		//__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : : "xmm5"); // xmm5 -- global accumulator
		xmm5 = _mm_setzero_si128();

		const size_t end=(size_t)(signature1+numberOf128BitWords);

		//__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
		xmm4 = xmm5;//_mm_load_si128(&xmm5);

		//for (n=0; n < numberOf128BitWords; n++) {
		do{
			//__asm__ volatile ("movdqa (%0), %%xmm0" : : "a" (signature1++) : "xmm0"); //slynen load data for XOR
			//		__asm__ volatile(
			//				"movdqa	  (%0), %%xmm0	\n"
			//"pxor      (%0), %%xmm0   \n" //slynen do XOR
			xmm0 = _mm_xor_si128 ( _mm_and_si128((__m128i)*signature1++, (__m128i)*mask), _mm_and_si128((__m128i)*signature2++, (__m128i)*mask)); //slynen load data for XOR and do XOR
			++mask;
			//				"movdqu    %%xmm0, %%xmm1	\n"
			xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
			//				"psrlw         $4, %%xmm1	\n"
			xmm1 = _mm_srl_epi16 (xmm1, shiftval);
			//				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
			xmm0 = _mm_and_si128 (xmm0, xmm6);
			//				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
			xmm1 = _mm_and_si128 (xmm1, xmm6);
			//				"movdqu    %%xmm7, %%xmm2	\n"
			xmm2 = xmm7;//_mm_loadu_si128(&xmm7);
			//				"movdqu    %%xmm7, %%xmm3	\n"	// get popcount
			xmm3 = xmm7;//_mm_loadu_si128(&xmm7);
			//				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
			xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
			//				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
			xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
			//				"paddb     %%xmm2, %%xmm4	\n"	// update local
			xmm4 = _mm_add_epi8(xmm4, xmm2);
			//				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
			xmm4 = _mm_add_epi8(xmm4, xmm3);
			//				:
			//				: "a" (buffer++)
			//				: "xmm0","xmm1","xmm2","xmm3","xmm4"
			//		);
		}while((size_t)signature1<end);
		// update global accumulator (two 32-bits counters)
		//	__asm__ volatile (
		//			/*"pxor	%xmm0, %xmm0		\n"*/
		//			"psadbw	%%xmm5, %%xmm4		\n"
		xmm4 = _mm_sad_epu8(xmm4, xmm5);
		//			"paddd	%%xmm4, %%xmm5		\n"
		xmm5 = _mm_add_epi32(xmm5, xmm4);
		//			:
		//			:
		//			: "xmm4","xmm5"
		//	);
		// finally add together 32-bits counters stored in global accumulator
	//	__asm__ volatile (
	//			"movhlps   %%xmm5, %%xmm0	\n"
		xmm0 = _mm_cvtps_epi32(_mm_movehl_ps(_mm_cvtepi32_ps(xmm0), _mm_cvtepi32_ps(xmm5))); //TODO fix with appropriate intrinsic
	//			"paddd     %%xmm5, %%xmm0	\n"
		xmm0 = _mm_add_epi32(xmm0, xmm5);
	//			"movd      %%xmm0, %%eax	\n"
		result = _mm_cvtsi128_si32 (xmm0);
	//			: "=a" (result) : : "xmm5","xmm0"
	//	);
		return result;
}

inline // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcntofXORed(const __m128i* signature1, const __m128i* signature2, const int numberOf128BitWords) {

	uint32_t result = 0;

		register __m128i xmm0;
		register __m128i xmm1;
		register __m128i xmm2;
		register __m128i xmm3;
		register __m128i xmm4;
		register __m128i xmm5;
		register __m128i xmm6;
		register __m128i xmm7;

		//__asm__ volatile ("movdqa (%0), %%xmm7" : : "a" (POPCOUNT_4bit) : "xmm7");
		xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
		//__asm__ volatile ("movdqa (%0), %%xmm6" : : "a" (MASK_4bit) : "xmm6");
		xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
		//__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : : "xmm5"); // xmm5 -- global accumulator
		xmm5 = _mm_setzero_si128();

		const size_t end=(size_t)(signature1+numberOf128BitWords);

		//__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
		xmm4 = xmm5;//_mm_load_si128(&xmm5);

		//for (n=0; n < numberOf128BitWords; n++) {
		do{
			//__asm__ volatile ("movdqa (%0), %%xmm0" : : "a" (signature1++) : "xmm0"); //slynen load data for XOR
			//		__asm__ volatile(
			//				"movdqa	  (%0), %%xmm0	\n"
			//"pxor      (%0), %%xmm0   \n" //slynen do XOR
			xmm0 = _mm_xor_si128 ( (__m128i)*signature1++, (__m128i)*signature2++); //slynen load data for XOR and do XOR
			//				"movdqu    %%xmm0, %%xmm1	\n"
			xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
			//				"psrlw         $4, %%xmm1	\n"
			xmm1 = _mm_srl_epi16 (xmm1, shiftval);
			//				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
			xmm0 = _mm_and_si128 (xmm0, xmm6);
			//				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
			xmm1 = _mm_and_si128 (xmm1, xmm6);
			//				"movdqu    %%xmm7, %%xmm2	\n"
			xmm2 = xmm7;//_mm_loadu_si128(&xmm7);
			//				"movdqu    %%xmm7, %%xmm3	\n"	// get popcount
			xmm3 = xmm7;//_mm_loadu_si128(&xmm7);
			//				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
			xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
			//				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
			xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
			//				"paddb     %%xmm2, %%xmm4	\n"	// update local
			xmm4 = _mm_add_epi8(xmm4, xmm2);
			//				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
			xmm4 = _mm_add_epi8(xmm4, xmm3);
			//				:
			//				: "a" (buffer++)
			//				: "xmm0","xmm1","xmm2","xmm3","xmm4"
			//		);
		}while((size_t)signature1<end);
		// update global accumulator (two 32-bits counters)
		//	__asm__ volatile (
		//			/*"pxor	%xmm0, %xmm0		\n"*/
		//			"psadbw	%%xmm5, %%xmm4		\n"
		xmm4 = _mm_sad_epu8(xmm4, xmm5);
		//			"paddd	%%xmm4, %%xmm5		\n"
		xmm5 = _mm_add_epi32(xmm5, xmm4);
		//			:
		//			:
		//			: "xmm4","xmm5"
		//	);
		// finally add together 32-bits counters stored in global accumulator
	//	__asm__ volatile (
	//			"movhlps   %%xmm5, %%xmm0	\n"
		xmm0 = _mm_cvtps_epi32(_mm_movehl_ps(_mm_cvtepi32_ps(xmm0), _mm_cvtepi32_ps(xmm5))); //TODO fix with appropriate intrinsic
	//			"paddd     %%xmm5, %%xmm0	\n"
		xmm0 = _mm_add_epi32(xmm0, xmm5);
	//			"movd      %%xmm0, %%eax	\n"
		result = _mm_cvtsi128_si32 (xmm0);
	//			: "=a" (result) : : "xmm5","xmm0"
	//	);
		return result;
}

inline // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcntofAND(const __m128i* signature1, const __m128i* signature2, const int numberOf128BitWords) {

	uint32_t result;

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));
	__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // xmm5 -- global accumulator

	result = 0;

	__asm__ volatile ("movdqu %xmm5, %xmm4"); // xmm4 -- local accumulator
	const size_t end=(size_t)(signature1+numberOf128BitWords);

	while((size_t)signature1<end){ // lestefan - small reduction of overhead
		__asm__ volatile ("movdqu (%%eax), %%xmm0" : : "a" (signature1++)); //slynen load data for xor
		__asm__ volatile(
				"pand      (%%eax), %%xmm0   \n" //slynen load data and do XOR
				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $4, %%xmm1	\n"
				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
				"movdqa    %%xmm7, %%xmm2	\n"
				"movdqa    %%xmm7, %%xmm3	\n"	// get popcount
				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
				"paddb     %%xmm2, %%xmm4	\n"	// update local
				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
				:
				: "a" (signature2++)
		);
	}

	__asm__ volatile (// update global accumulator (two 32-bits counters)
			"psadbw	%xmm5, %xmm4		\n"
			"paddd	%xmm4, %xmm5		\n"
	);

	__asm__ volatile (// finally add together 32-bits counters stored in global accumulator
			"movhlps   %%xmm5, %%xmm0	\n"
			"paddd     %%xmm5, %%xmm0	\n"
			"movd      %%xmm0, %%eax	\n"
			: "=a" (result)
	);
	return result;
}


__inline__ // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcntofXORed_intel(const __m128i* signature1, const __m128i* signature2, const int numberOf128BitWords) {
	//static const char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
	//static const uint8_t POPCOUNT_4bit[16] __attribute__((aligned(16))) = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

	//const U_INT8T_ALIAS* buffer1 = (U_INT8T_ALIAS*)(signature1);
	//const U_INT8T_ALIAS* buffer2 = (U_INT8T_ALIAS*)(signature2);
	uint32_t result = 0;

	register __m128i xmm0;
	register __m128i xmm1;
	register __m128i xmm2;
	register __m128i xmm3;
	register __m128i xmm4;
	register __m128i xmm5;
	register __m128i xmm6;
	register __m128i xmm7;

	//__asm__ volatile ("movdqa (%0), %%xmm7" : : "a" (POPCOUNT_4bit) : "xmm7");
	xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
	//__asm__ volatile ("movdqa (%0), %%xmm6" : : "a" (MASK_4bit) : "xmm6");
	xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
	//__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : : "xmm5"); // xmm5 -- global accumulator
	xmm5 = _mm_setzero_si128();

	const size_t end=(size_t)(signature1+numberOf128BitWords);

	//__asm__ volatile ("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator
	xmm4 = xmm5;//_mm_load_si128(&xmm5);

	//for (n=0; n < numberOf128BitWords; n++) {
	do{
		//__asm__ volatile ("movdqa (%0), %%xmm0" : : "a" (signature1++) : "xmm0"); //slynen load data for XOR
		//		__asm__ volatile(
		//				"movdqa	  (%0), %%xmm0	\n"
		//"pxor      (%0), %%xmm0   \n" //slynen do XOR
		xmm0 = _mm_xor_si128 ( (__m128i)*signature1++, (__m128i)*signature2++); //slynen load data for XOR and do XOR
		//				"movdqu    %%xmm0, %%xmm1	\n"
		xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
		//				"psrlw         $4, %%xmm1	\n"
		xmm1 = _mm_srl_epi16 (xmm1, shiftval);
		//				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
		xmm0 = _mm_and_si128 (xmm0, xmm6);
		//				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
		xmm1 = _mm_and_si128 (xmm1, xmm6);
		//				"movdqu    %%xmm7, %%xmm2	\n"
		xmm2 = xmm7;//_mm_loadu_si128(&xmm7);
		//				"movdqu    %%xmm7, %%xmm3	\n"	// get popcount
		xmm3 = xmm7;//_mm_loadu_si128(&xmm7);
		//				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
		xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
		//				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
		xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
		//				"paddb     %%xmm2, %%xmm4	\n"	// update local
		xmm4 = _mm_add_epi8(xmm4, xmm2);
		//				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
		xmm4 = _mm_add_epi8(xmm4, xmm3);
		//				:
		//				: "a" (buffer++)
		//				: "xmm0","xmm1","xmm2","xmm3","xmm4"
		//		);
	}while((size_t)signature1<end);
	// update global accumulator (two 32-bits counters)
	//	__asm__ volatile (
	//			/*"pxor	%xmm0, %xmm0		\n"*/
	//			"psadbw	%%xmm5, %%xmm4		\n"
	xmm4 = _mm_sad_epu8(xmm4, xmm5);
	//			"paddd	%%xmm4, %%xmm5		\n"
	xmm5 = _mm_add_epi32(xmm5, xmm4);
	//			:
	//			:
	//			: "xmm4","xmm5"
	//	);
	// finally add together 32-bits counters stored in global accumulator
//	__asm__ volatile (
//			"movhlps   %%xmm5, %%xmm0	\n"
	xmm0 = _mm_cvtps_epi32(_mm_movehl_ps(_mm_cvtepi32_ps(xmm0), _mm_cvtepi32_ps(xmm5))); //TODO fix with appropriate intrinsic
//			"paddd     %%xmm5, %%xmm0	\n"
	xmm0 = _mm_add_epi32(xmm0, xmm5);
//			"movd      %%xmm0, %%eax	\n"
	result = _mm_cvtsi128_si32 (xmm0);
//			: "=a" (result) : : "xmm5","xmm0"
//	);
	return result;
}

__inline__ // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
__m128i ssse3_popcntofXORed_8bit(const __m128i& signature1, const __m128i& signature2) {

	register __m128i xmm0;
	register __m128i xmm1;
	register __m128i xmm2;
	register __m128i xmm3;
	register __m128i xmm4;
	register __m128i xmm6;
	register __m128i xmm7;

	xmm7 = _mm_load_si128 ((__m128i *)POPCOUNT_4bit);
	xmm6 = _mm_load_si128 ((__m128i *)MASK_4bit);
	xmm4 = _mm_setzero_si128();

		xmm0 = _mm_xor_si128 ( signature1, signature2);
		xmm1 = xmm0;//_mm_loadu_si128(&xmm0);
		xmm1 = _mm_srl_epi16 (xmm1, shiftval);
		xmm0 = _mm_and_si128 (xmm0, xmm6);
		xmm1 = _mm_and_si128 (xmm1, xmm6);
		xmm2 = xmm7;
		xmm3 = xmm7;
		xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
		xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
		xmm4 = _mm_add_epi8(xmm4, xmm2);
		xmm4 = _mm_add_epi8(xmm4, xmm3);
	
	return xmm4;
}

__inline__ // - SSSE3 - better alorithm, minimized psadbw usage - adapted from http://wm.ite.pl/articles/sse-popcount.html
uint32_t ssse3_popcntofXORed(const __m128i* signature1,const __m128i* signature2, int numberOf128BitWords, uint32_t max) {

	uint32_t result;

	__asm__ volatile ("movdqu (%%eax), %%xmm7" : : "a" (POPCOUNT_4bit));
	__asm__ volatile ("movdqu (%%eax), %%xmm6" : : "a" (MASK_4bit));

	result = 0;

	__asm__ volatile ("pxor %xmm4, %xmm4"); // xmm4 -- local accumulator
	const size_t end=(size_t)(signature1+numberOf128BitWords);

	while((size_t)signature1<end){ // lestefan - small reduction of overhead
		__asm__ volatile ("movdqu (%%eax), %%xmm0" : : "a" (signature1++)); //slynen load data for xor
		__asm__ volatile(
				"pxor      (%%eax), %%xmm0   \n" //slynen load data and XOR
				"movdqa    %%xmm0, %%xmm1	\n"
				"psrlw         $4, %%xmm1	\n"
				"pand      %%xmm6, %%xmm0	\n"	// xmm0 := lower nibbles
				"pand      %%xmm6, %%xmm1	\n"	// xmm1 := higher nibbles
				"movdqa    %%xmm7, %%xmm2	\n"
				"movdqa    %%xmm7, %%xmm3	\n"	// get popcount
				"pshufb    %%xmm0, %%xmm2	\n"	// for all nibbles
				"pshufb    %%xmm1, %%xmm3	\n"	// using PSHUFB
				"paddb     %%xmm2, %%xmm4	\n"	// update local
				"paddb     %%xmm3, %%xmm4	\n"	// accumulator
				:
				: "a" (signature2++)
		);
		__asm__ volatile ("pxor    %%xmm5, %%xmm5" : : ); // slynen reset needed inside loop for premature break
		__asm__ volatile (// update global accumulator (two 32-bits counters)
				"movdqu	%xmm5, %xmm0		\n"
				"psadbw	%xmm0, %xmm4		\n"
				"paddd	%xmm4, %xmm5		\n"
		);

		__asm__ volatile (// finally add together 32-bits counters stored in global accumulator
				"movhlps   %%xmm5, %%xmm0	\n"
				"paddd     %%xmm5, %%xmm0	\n"
				"movd      %%xmm0, %%eax	\n"
				: "=a" (result)
		);
		if(result>max) //that takes about 5% of the time
			break;

	}
	return result;
}

}

#endif /* POPCNT_H_ */
