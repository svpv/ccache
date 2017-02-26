// Copyright (c) 2017 Alexey Tourbin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef RAWMEMCHR2_H
#define RAWMEMCHR2_H

#ifdef __SSE2__
#include <emmintrin.h>

#ifdef __GNUC__
#define ffs __builtin_ffs
#else
#include <strings.h>
#endif

// Find the first instance of either c1 or c2.
// The search is unbounded; therefore, the caller should
// typically install a sentinel (c1 or c2) at the end of
// the buffer.  Following the sentinel, the caller should
// also provide 15 bytes of padding.
static inline void *rawmemchr2(const void *s, int c1, int c2)
{
	__m128i w1 = _mm_set1_epi8(c1);
	__m128i w2 = _mm_set1_epi8(c2);
	unsigned short mask;
	while (1) {
		__m128i w = _mm_loadu_si128(s);
		__m128i cmp1 = _mm_cmpeq_epi8(w, w1);
		__m128i cmp2 = _mm_cmpeq_epi8(w, w2);
		__m128i cmp = _mm_or_si128(cmp1, cmp2);
		mask = _mm_movemask_epi8(cmp);
		if (mask)
			break;
		s = (char *) s + 16;
	}
	return (char *) s + ffs(mask) - 1;
}

#ifdef __GNUC__
#undef ffs
#endif

#else // no SSE2

// Constants for haszero().
#if ~0UL > 0xffffffffUL
#define C01 0x0101010101010101UL
#define C80 0x8080808080808080UL
#else
#define C01 0x01010101UL
#define C80 0x80808080UL
#endif

// Determine if a word has a zero byte.  Dates back to 1987,
// see the "Bit Twiddling Hacks" page by Sean Eron Anderson.
#define haszero(v) (((v) - C01) & ~(v) & C80)

// A helper function which should not be called directly.
static
// Function calls are somewhat expensive on x86, while the code
// is actually smaller, due to unaligned reads.
#ifdef __i386__
inline
#endif
void *rawmemchr2w(const void *s, int c1, int c2,
		  unsigned long c1w, unsigned long c2w)
{
	// Cast away const, only to match the return type.
	unsigned char *p = (void *) s;

	// Unless the platform supports fast unaligned reads:
	// check one byte at a time, until the input is aligned.
#ifndef __i386__
	switch ((unsigned long) s % sizeof(unsigned long)) {
#if ~0UL > 0xffffffffUL
	case 7: if (*p == c1 || *p == c2) return p; p++;
	case 6: if (*p == c1 || *p == c2) return p; p++;
	case 5: if (*p == c1 || *p == c2) return p; p++;
	case 4: if (*p == c1 || *p == c2) return p; p++;
#endif
	case 3: if (*p == c1 || *p == c2) return p; p++;
	case 2: if (*p == c1 || *p == c2) return p; p++;
	case 1: if (*p == c1 || *p == c2) return p; p++;
	}
#endif
	// Will now scan in words.
	const unsigned long *w = (void *) p;

	// An iteration: if a word has c1 or c2, the corresponding
	// byte will be 0 after XOR.  We could further combine two
	// haszero() tests and factor out the last C80 step, but
	// this does not seem to improve performance.
#define ITER						\
	{						\
		unsigned long w1 = *w ^ c1w;		\
		unsigned long w2 = *w ^ c2w;		\
		if (haszero(w1) || haszero(w2))		\
			break;				\
		w++;					\
	}

	// Unroll the loop.
	while (1) {
		ITER ITER ITER ITER
	}
#undef ITER

	// Handle the remaining bytes.  We could use ffs() or clz()
	// here (the latter on big endian platforms) to determine
	// the matching byte, but this would require a more elaborate
	// ITER step.  Overall, there seems to be no improvement.
	p = (void *) w;
	if (p[0] == c1 || p[0] == c2) return p + 0;
	if (p[1] == c1 || p[1] == c2) return p + 1;
	if (p[2] == c1 || p[2] == c2) return p + 2;
#if ~0UL > 0xffffffffUL
	if (p[3] == c1 || p[3] == c2) return p + 3;
	if (p[4] == c1 || p[4] == c2) return p + 4;
	if (p[5] == c1 || p[5] == c2) return p + 5;
	if (p[6] == c1 || p[6] == c2) return p + 6;
	return p + 7;
#else
	return p + 3;
#endif
}

#undef C01
#undef C80
#undef haszero

static inline void *rawmemchr2(void const *s, int c1, int c2)
{
	// This trick makes a "word" out of a character
	// by "repeating" it 4 or 8 times.  Since c1 and c2
	// are often constants, the trick works at compile
	// time and saves a few instructions.
	unsigned long c1w = ~0UL / 255 * c1;
	unsigned long c2w = ~0UL / 255 * c2;
	return rawmemchr2w(s, c1, c2, c1w, c2w);
}

#endif
#endif
