/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-202 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "operations.h"
#include "../../detail/psyconf.h"

#if defined PSYCLE_USE_SSE && defined DIVERSALIS__CPU__X86__SSE
#include <string.h>
#include "alignedalloc.h"
#include <xmmintrin.h>

#define is_aligned(POINTER, BYTE_COUNT) \
	(((uintptr_t)(const void*)(POINTER)) % (16) == 0)

static size_t alignment = 16;
static psy_dsp_Operations noopt;

static void* dsp_memory_alloc(size_t count, size_t size);
static void dsp_memory_dealloc(void* address);
static void dsp_add(float *src, float *dst, uintptr_t num, float vol);
static void dsp_mul(float *dst, uintptr_t num, float mul);
static void dsp_movmul(float *src, float *dst, uintptr_t num, float mul);
static void dsp_clear(float *dst, uintptr_t num);
static float* dsp_crop(float* src, uintptr_t offset, uintptr_t num);
static void dsp_interleave(float* dst, float* left, float* right, uintptr_t num);
static void dsp_erase_all_nans_infinities_and_denormals(float* dst,
		uintptr_t num);
static void erase_all_nans_infinities_and_denormals(float* sample);
static float dsp_maxvol(const float* pSamples, uintptr_t numSamples);
static void dsp_accumulate(double* accumleft,
	double* accumright,
	const float* __restrict pSamplesL,
	const float* __restrict pSamplesR, int count);
static void dsp_reverse(float* dst, uintptr_t num);

void psy_dsp_sse2_init(psy_dsp_Operations* self)
{	
	psy_dsp_noopt_init(&noopt);
	self->memory_alloc = dsp_memory_alloc;
	self->memory_dealloc = dsp_memory_dealloc;
	self->add = dsp_add;
	self->mul = dsp_mul;
	self->movmul = dsp_movmul;
	self->clear = dsp_clear;
	self->crop = dsp_crop;
	self->interleave = dsp_interleave;
	self->erase_all_nans_infinities_and_denormals = dsp_erase_all_nans_infinities_and_denormals;
	self->maxvol = dsp_maxvol;
	self->accumulate = dsp_accumulate;
	self->reverse = dsp_reverse;
}

void* dsp_memory_alloc(size_t count, size_t size)
{	
	if ((count & 3) != 0) {
		count += (4 - (count % 4));
	}
	return psy_dsp_aligned_memory_alloc(alignment, count, size);
}

void dsp_memory_dealloc(void* address)
{
	psy_dsp_aligned_memory_dealloc(address);
}

void dsp_add(float *src, float *dst, uintptr_t num, float vol)
{
	if (is_aligned(dst, 16) && (num % 4 == 0)) {
/* omp parallel causes extreme high cpu usage
#if defined DIVERSALIS__COMPILER__GNU
		num += 3;
		num >>= 2;
		typedef float vec __attribute__((vector_size(4 * sizeof(float))));
		const vec vol_vec = { vol, vol, vol, vol };
		const vec* vsrc = (const vec*)(src);
		vec* vdst = (vec*)(dst);
#pragma omp parallel for // with gcc, build with the -fopenmp flag
		for (int i = 0; i < num; ++i) vdst[i] += vsrc[i] * vol_vec;
#el*/
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		const __m128 volps = _mm_set_ps1(vol);
		const __m128* psrc = (const __m128*)src;
		__m128* pdst = (__m128*)dst;
		uintptr_t i;
		num += 3;
		num >>= 2;
		for (i = 0; i < num; ++i) pdst[i] = _mm_add_ps(pdst[i], _mm_mul_ps(psrc[i], volps));
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		__asm
		{
			movss xmm2, vol
			shufps xmm2, xmm2, 0H
			mov esi, src
			mov edi, dst
			mov eax, [num]
			LOOPSTART:
			cmp eax, 0
				jle END
				movaps xmm0, [esi]
				movaps xmm1, [edi]
				mulps xmm0, xmm2
				addps xmm0, xmm1
				movaps[edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
		}
#else
		noopt.add(src, dst, num, vol);
#endif
	} else {
		noopt.add(src, dst, num, vol);
	}
}
	
void dsp_mul(float *dst, uintptr_t num, float multi)
{	
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	if (is_aligned(dst, 16) && (num % 4 == 0)) {
		const __m128 volps = _mm_set_ps1(multi);
		__m128* pdst = (__m128*)dst;
		while (num > 0)
		{
			*pdst = _mm_mul_ps(*pdst, volps);
			pdst++;
			num -= 4;
		}
	} else {
		noopt.mul(dst, num, multi);
	}
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	// This code assumes aligned memory (to 16) and assigned by powers of 4!
	if (is_aligned(dst, 16) && (num % 4 == 0)) {
		__asm
		{
			movss xmm2, multi
			shufps xmm2, xmm2, 0H
			mov edi, dst
			mov eax, [num]
			LOOPSTART:
			cmp eax, 0
				jle END
				movaps xmm0, [edi]
				mulps xmm0, xmm2
				movaps[edi], xmm0
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
		}
	} else {
		noopt.mul(dst, num, multi);
	}
#else
	noopt.mul(dst, num, multi);
#endif	
}
	
void dsp_movmul(float *src, float *dst, uintptr_t num, float multi)
{	
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	const __m128 volps = _mm_set_ps1(multi);
	const __m128* psrc = (__m128*)src;
	if (is_aligned(dst, 16)) {
		while (num > 3)
		{
			_mm_storeu_ps(dst, _mm_mul_ps(*psrc, volps));
			psrc++;
			dst += 4;
			num -= 4;
		}
		if (num > 0) {
			noopt.movmul(src, dst, num, multi);
		}
	} else {
		noopt.movmul(src, dst, num, multi);
	}
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	if (is_aligned(dst, 16)) {
		__asm
		{
			movss xmm2, multi
			shufps xmm2, xmm2, 0H
			mov esi, src
			mov edi, dst
			mov eax, [num]
			LOOPSTART:
			cmp eax, 3
				jle END
				movaps xmm0, [esi]
				mulps xmm0, xmm2
				movups[edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
		}
		if (num > 0) {
			noopt.movmul(src, dst, num, multi);
		}
	} else {
		__asm
		{
			movss xmm2, multi
			shufps xmm2, xmm2, 0H
			mov esi, src
			mov edi, dst
			mov eax, [num]
			LOOPSTART:
			cmp eax, 0
				jle END
				movaps xmm0, [esi]
				mulps xmm0, xmm2
				movaps[edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
		}
	}
#else
	noopt.movmul(src, dst, num, multi);
#endif
}
	
void dsp_clear(float* dst, uintptr_t num)
{
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	if (is_aligned(dst, 16) && (num % 4 == 0)) {
		const __m128 zeroval = _mm_set_ps1(0.0f);
		while (num > 0)
		{
			_mm_store_ps(dst, zeroval);
			dst += 4;
			num -= 4;
		}
	} else {
		noopt.clear(dst, num);
	}
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	// This code assumes aligned memory (to 16) and assigned by powers of 4!
	if (is_aligned(dst, 16) && (num % 4 == 0)) {
		__asm
		{
			xorps xmm0, xmm0
			mov edi, dst
			mov eax, [num]
			LOOPSTART:
			cmp eax, 0
				jle END
				movaps[edi], xmm0
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
		}
	} else {
		noopt.clear(dst, num);
	}
#else
	noopt.clear(dst, num);
#endif
}

float* dsp_crop(float* src, uintptr_t offset, uintptr_t num)
{
	float* rv;

	if (num > 0) {
		uintptr_t i;
		float* frame;
		float* dst;

		dst = rv = dsp_memory_alloc(num, sizeof(float));
		for (frame = src + offset, i = 0; i < num; ++i) {
			*(dst++) = *(frame++);
		}
	} else {
		rv = NULL;
	}
	dsp.memory_dealloc(src);
	return rv;
}

void dsp_interleave(float* dst, float* left, float* right, uintptr_t num)
{
	uintptr_t i;
	--dst;
	--left;
	--right;	
	i = num;
	do {
		++dst;
		++left;
		++right;
		*dst = *left;
		++dst;
		*dst = *right;
	}
	while (--i);
}

void dsp_erase_all_nans_infinities_and_denormals(float* dst,
		uintptr_t num) {
	uintptr_t i;

	for(i = 0; i < num; ++i) {
		erase_all_nans_infinities_and_denormals(&dst[i]);
	}
}

/// Cure for malicious samples
/// Type : Filters Denormals, NaNs, Infinities
/// References : Posted by urs[AT]u-he[DOT]com
void erase_all_nans_infinities_and_denormals(float* sample) {
	#if !defined DIVERSALIS__CPU__X86
		// just do nothing.. not crucial for other archs ?
	#else
	uint32_t exponent_mask;
	uint32_t exponent;
	uint32_t not_nan_nor_infinity;
	uint32_t not_denormal;
		union {
			float sample;
			uint32_t bits;
		} u;
		u.sample = *sample;
		exponent_mask = 0x7f800000;
		exponent = u.bits & exponent_mask;
		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		not_nan_nor_infinity = exponent < exponent_mask;

		// exponent > 0 is 0 if denormalized, otherwise 1
		not_denormal = exponent > 0;

		u.bits *= not_nan_nor_infinity & not_denormal;
		*sample = u.sample;
	#endif
}

//same method, for a single buffer (allowing to calculate max for each buffer). samples need to be aligned by 16 in optimized paths.
float dsp_maxvol(const float* pSamples, uintptr_t numSamples)
{
	if (!is_aligned(pSamples, 16) || (numSamples % 4 != 0)) {
		return noopt.maxvol(pSamples, numSamples);
	} else {
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 minVol = _mm_set_ps1(0.0f);
		__m128 maxVol = _mm_set_ps1(0.0f);
		const __m128* psrc = (const __m128*)pSamples;
		while (numSamples > 0) {
			maxVol = _mm_max_ps(maxVol, *psrc);
			minVol = _mm_min_ps(minVol, *psrc);
			psrc++;
			numSamples -= 4;
		}
		__m128 highTmp = _mm_movehl_ps(maxVol, maxVol);
		maxVol = _mm_max_ps(maxVol, highTmp);
		highTmp = _mm_move_ss(highTmp, maxVol);
		maxVol = _mm_shuffle_ps(maxVol, highTmp, 0x11);
		maxVol = _mm_max_ps(maxVol, highTmp);

		__m128 lowTmp = _mm_movehl_ps(minVol, minVol);
		minVol = _mm_max_ps(minVol, lowTmp);
		lowTmp = _mm_move_ss(lowTmp, minVol);
		minVol = _mm_shuffle_ps(minVol, lowTmp, 0x11);
		minVol = _mm_max_ps(minVol, lowTmp);

		__m128 minus1 = _mm_set_ps1(-1.0f);
		minVol = _mm_mul_ss(minVol, minus1);
		//beware: -0.0f and 0.0f are supposed to be the same number, -0.0f > 0.0f returns false, just because 0.0f == 0.0f returns true
		maxVol = _mm_max_ps(minVol, maxVol);
		float result;
		_mm_store_ss(&result, maxVol);
		return result;
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// If anyone knows better assembler than me improve this variable utilization:
		float volmax = 0.0f, volmin = 0.0f;
		float* volmaxb = &volmax, * volminb = &volmin;
		__asm
		{
			// we store the max in xmm0 and the min in xmm1
			xorps xmm0, xmm0
			xorps xmm1, xmm1
			mov esi, [pSamples]
			mov eax, [numSamples]
			// Loop does: get the 4 max values and 4 min values in xmm0 and xmm1 respct.
			LOOPSTART:
			cmp eax, 0
				jle END
				maxps xmm0, [esi]
				minps xmm1, [esi]
				add esi, 10H
				sub eax, 4
				jmp LOOPSTART
				END :
			// to finish, get the max and of each of the four values.
			// put 02 and 03 to 20 and 21
			movhlps xmm2, xmm0
				// find max of 00 and 20 (02) and of 01 and 21 (03)
				maxps xmm0, xmm2
				// put 00 (result of max(00,02)) to 20
				movss xmm2, xmm0
				// put 01 (result of max(01,03)) into 00 (that's the only one we care about)
				shufps xmm0, xmm2, 11H
				// and find max of 00 (01) and 20 (00)
				maxps xmm0, xmm2

				movhlps xmm2, xmm1
				minps xmm1, xmm2
				movss xmm2, xmm1
				shufps xmm1, xmm2, 11H
				minps xmm1, xmm2

				mov edi, volmaxb
				movss[edi], xmm0
				mov edi, volminb
				movss[edi], xmm1
		}
		volmin *= -1.0f;
		//beware: -0.0f and 0.0f are supposed to be the same number, -0.0f > 0.0f returns false, just because 0.0f == 0.0f returns true
		return (volmin > volmax) ? volmin : volmax;
#else
#endif
	}
}

void dsp_accumulate(double* accumleft,
	double* accumright,
	const float* __restrict pSamplesL,
	const float* __restrict pSamplesR,
	int count)
{		
	if (count == 0)
	{
		return;
	}
	if ((count % 4 != 0) || !is_aligned(pSamplesL, count) || is_aligned(pSamplesR, count)) {
		noopt.accumulate(accumleft, accumright, pSamplesL, pSamplesR, count);
	} else {
		float result;

		__m128 acleftvec = _mm_set_ps1(0.0f);
		__m128 acrightvec = _mm_set_ps1(0.0f);
		const __m128* psrcl = (const __m128*)pSamplesL;
		const __m128* psrcr = (const __m128*)pSamplesR;
		while (count > 3) {
			acleftvec = _mm_add_ps(acleftvec, _mm_mul_ps(*psrcl, *psrcl));
			acrightvec = _mm_add_ps(acrightvec, _mm_mul_ps(*psrcr, *psrcr));
			psrcl++;
			psrcr++;
			count -= 4;
		}
		acleftvec = _mm_add_ps(acleftvec, _mm_movehl_ps(acleftvec, acleftvec)); // add (0= 0+2 and 1=1+3 thanks to the move)
		acleftvec = _mm_add_ss(acleftvec, _mm_shuffle_ps(acleftvec, acleftvec, 0x11)); // add (0 = 0+1 thanks to the shuffle)
		_mm_store_ss(&result, acleftvec); //get the position 0
		*accumleft += result;
		acrightvec = _mm_add_ps(acrightvec, _mm_movehl_ps(acrightvec, acrightvec)); // add (0= 0+2 and 1=1+3 thanks to the move)
		acrightvec = _mm_add_ss(acrightvec, _mm_shuffle_ps(acrightvec, acrightvec, 0x11)); // add (0 = 0+1 thanks to the shuffle)
		_mm_store_ss(&result, acrightvec); //get the position 0
		*accumright += result;
		if (count > 0) {
			const float* pL = (const float*)psrcl;
			const float* pR = (const float*)psrcr;
			while (count--) {
				*accumleft += (*pL * *pL); pL++;
				*accumright += (*pR * *pR); pR++;
			}
		}
	}
}

void dsp_reverse(float* dst, uintptr_t num)
{
	noopt.reverse(dst, num);
}

#else

#include <string.h>

void psy_dsp_sse2_init(psy_dsp_Operations* self)
{
	memset(self, 0, sizeof(psy_dsp_Operations));
}

#endif
