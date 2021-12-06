//+[TCCQB] QuickBoot Image Decompress Algorithm
/*
   LZ4 - Fast LZ compression algorithm
   Copyright (C) 2011-2012, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 homepage : http://fastcompression.blogspot.com/p/lz4.html
   - LZ4 source repository : http://code.google.com/p/lz4/
*/

//**************************************
// Includes
//**************************************
#include <debug.h>
#include <arch/arm.h>
#include <string.h>


//**************************************
// Tuning parameters
//**************************************
// COMPRESSIONLEVEL :
// Increasing this value improves compression ratio
// Lowering this value reduces memory usage
// Reduced memory usage typically improves speed, due to cache effect (ex : L1 32KB for Intel, L1 64KB for AMD)
// Memory usage formula : N->2^(N+2) Bytes (examples : 12 -> 16KB ; 17 -> 512KB)
#define COMPRESSIONLEVEL 12

// NOTCOMPRESSIBLE_CONFIRMATION :
// Decreasing this value will make the algorithm skip faster data segments considered "incompressible"
// This may decrease compression ratio dramatically, but will be faster on incompressible data
// Increasing this value will make the algorithm search more before declaring a segment "incompressible"
// This could improve compression a bit, but will be slower on incompressible data
// The default value (6) is recommended
#define NOTCOMPRESSIBLE_CONFIRMATION 6

// LZ4_COMPRESSMIN :
// Compression function will *fail* if it is not successful at compressing input by at least LZ4_COMPRESSMIN bytes
// Since the compression function stops working prematurely, it results in a speed gain
// The output however is unusable. Compression function result will be zero.
// Default : 0 = disabled
#define LZ4_COMPRESSMIN 0

// BIG_ENDIAN_NATIVE_BUT_INCOMPATIBLE :
// This will provide a boost to performance for big endian cpu, but the resulting compressed stream will be incompatible with little-endian CPU.
// You can set this option to 1 in situations where data will stay within closed environment
// This option is useless on Little_Endian CPU (such as x86)
//#define BIG_ENDIAN_NATIVE_BUT_INCOMPATIBLE 1



//**************************************
// CPU Feature Detection
//**************************************

// Little Endian or Big Endian ?
// Note : overwrite the below #define if you know your architecture endianess
// Little Endian assumed. PDP Endian and other very rare endian format are unsupported.


// Unaligned memory access is automatically enabled for "common" CPU, such as x86.
// For others CPU, the compiler will be more cautious, and insert extra code to ensure aligned access is respected
// If you know your target CPU supports unaligned memory access, you may want to force this option manually to improve performance
#if defined(__ARM_FEATURE_UNALIGNED)
#define LZ4_FORCE_UNALIGNED_ACCESS 1
#endif


//**************************************
// Compiler Options
//**************************************
//#if __STDC_VERSION__ >= 199901L // C99
/* "restrict" is a known keyword */
//#else
#define restrict // Disable restrict
//#endif

#define lz4_bswap16(x) ((unsigned short int) ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8)))


#undef likely
#undef unlikely
#define likely(expr)     (__builtin_expect((expr) != 0, 1))
#define unlikely(expr)   (__builtin_expect((expr) != 0, 0))


//**************************************
// Basic Types
//**************************************
#define BYTE	uint8_t
#define U16		uint16_t
#define U32		uint32_t
#define S32		int32_t
#define U64		uint64_t

#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(push, 1)
#endif

typedef struct _U16_S { U16 v; } U16_S;
typedef struct _U32_S { U32 v; } U32_S;
typedef struct _U64_S { U64 v; } U64_S;

#ifndef LZ4_FORCE_UNALIGNED_ACCESS
#pragma pack(pop)
#endif

#if 1
#define A64(x) (((U64_S *)(x))->v)
#define A32(x) (((U32_S *)(x))->v)
#define A16(x) (((U16_S *)(x))->v)
#else
#define A64(x) ((*(U64_S *)(x)))
#define A32(x) ((*(U32_S *)(x)))
#define A16(x) ((*(U16_S *)(x)))
#endif


//**************************************
// Constants
//**************************************
#define MINMATCH 4

#define HASH_LOG COMPRESSIONLEVEL
#define HASHTABLESIZE (1 << HASH_LOG)
#define HASH_MASK (HASHTABLESIZE - 1)

#define SKIPSTRENGTH (NOTCOMPRESSIBLE_CONFIRMATION>2?NOTCOMPRESSIBLE_CONFIRMATION:2)
#define STACKLIMIT 13
#define HEAPMODE (HASH_LOG>STACKLIMIT)  // Defines if memory is allocated into the stack (local variable), or into the heap (malloc()).
#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH+MINMATCH)
#define MINLENGTH (MFLIMIT+1)

#define MAXD_LOG 16
#define MAX_DISTANCE ((1 << MAXD_LOG) - 1)

#define ML_BITS 4
#define ML_MASK ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)


//**************************************
// Architecture-specific macros
//**************************************
#define STEPSIZE 4
#define UARCH U32
#define AARCH A32
#define LZ4_COPYSTEP(s,d)		A32(d) = A32(s); d+=4; s+=4;
#define LZ4_COPYPACKET(s,d)		LZ4_COPYSTEP(s,d); LZ4_COPYSTEP(s,d);
#define LZ4_SECURECOPY			LZ4_WILDCOPY
#define HTYPE const BYTE*
#define INITBASE(base)			const int base = 0

#define LZ4_READ_LITTLEENDIAN_16(d,s,p) { d = (s) - A16(p); }
#define LZ4_WRITE_LITTLEENDIAN_16(p,v)  { A16(p) = v; p+=2; }


//**************************************
// Local structures
//**************************************


//**************************************
// Macros
//**************************************
#define LZ4_HASH_FUNCTION(i)	(((i) * 2654435761U) >> ((MINMATCH*8)-HASH_LOG))
#define LZ4_HASH_VALUE(p)		LZ4_HASH_FUNCTION(A32(p))
#define LZ4_WILDCOPY(s,d,e)		do { LZ4_COPYPACKET(s,d) } while (d<e);
#define LZ4_BLINDCOPY(s,d,l)	{ BYTE* e=(d)+l; LZ4_WILDCOPY(s,d,e); d=e; }


//****************************
// Private functions
//****************************
inline static int LZ4_NbCommonBytes (register U32 val)
{
    return (__builtin_ctz(val) >> 3);
}


//****************************
// Public functions
//****************************

// Note : this function is valid only if isize < LZ4_64KLIMIT
#define HASHLOG64K (HASH_LOG+1)
#define HASH64KTABLESIZE (1U<<HASHLOG64K)
#define LZ4_HASH64K_FUNCTION(i)	(((i) * 2654435761U) >> ((MINMATCH*8)-HASHLOG64K))
#define LZ4_HASH64K_VALUE(p)	LZ4_HASH64K_FUNCTION(A32(p))


//****************************
// Decompression functions
//****************************

// Note : The decoding functions LZ4_uncompress() and LZ4_uncompress_unknownOutputSize()
//		are safe against "buffer overflow" attack type.
//		They will never write nor read outside of the provided output buffers.
//      LZ4_uncompress_unknownOutputSize() also insures that it will never read outside of the input buffer.
//		A corrupted input will produce an error result, a negative int, indicating the position of the error within input stream.

int LZ4_uncompress(const char* source,
				 char* dest,
				 int osize)
{
	// Local Variables
	const BYTE* restrict ip = (const BYTE*) source;
	const BYTE* restrict ref;

	BYTE* restrict op = (BYTE*) dest;
	BYTE* const oend = op + osize;
	BYTE* cpy;

	BYTE token;

	int	len, length;
	size_t dec[] ={0, 3, 2, 3, 0, 0, 0, 0};


	// Main Loop
	while (1)
	{
		// get runlength
		token = *ip++;
		if ((length=(token>>ML_BITS)) == RUN_MASK)  { for (;(len=*ip++)==255;length+=255){} length += len; }

		// copy literals
		cpy = op+length;
		if (unlikely(cpy>oend-COPYLENGTH))
		{
			if (cpy > oend) goto _output_error;          // Error : request to write beyond destination buffer
			memcpy(op, ip, length);
			ip += length;
			break;    // Necessarily EOF
		}
		LZ4_WILDCOPY(ip, op, cpy); ip -= (op-cpy); op = cpy;

		// get offset
		LZ4_READ_LITTLEENDIAN_16(ref,cpy,ip); ip+=2;
		if (ref < (BYTE* const)dest) goto _output_error;   // Error : offset create reference outside destination buffer

		// get matchlength
		if ((length=(token&ML_MASK)) == ML_MASK) { for (;*ip==255;length+=255) {ip++;} length += *ip++; }

		// copy repeated sequence
		if (unlikely(op-ref<STEPSIZE))
		{
			const int dec2 = 0;

			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			ref -= dec[op-ref];
			A32(op)=A32(ref); op += STEPSIZE-4;
			ref -= dec2;
		} else { LZ4_COPYSTEP(ref,op); }
		cpy = op + length - (STEPSIZE-4);
		if (cpy>oend-COPYLENGTH)
		{
			if (cpy > oend) goto _output_error;             // Error : request to write beyond destination buffer
			LZ4_SECURECOPY(ref, op, (oend-COPYLENGTH));
			while(op<cpy) *op++=*ref++;
			op=cpy;
			if (op == oend) break;    // Check EOF (should never happen, since last 5 bytes are supposed to be literals)
			continue;
		}
		LZ4_SECURECOPY(ref, op, cpy);
		op=cpy;		// correction
	}

	// end of decoding
	return (int) (((char*)ip)-source);

	// write overflow error detected
_output_error:
	return (int) (-(((char*)ip)-source));
}


int LZ4_uncompress_unknownOutputSize(
				const char* source,
				char* dest,
				int isize,
				int maxOutputSize)
{
	// Local Variables
	const BYTE* restrict ip = (const BYTE*) source;
	const BYTE* const iend = ip + isize;
	const BYTE* restrict ref;

	BYTE* restrict op = (BYTE*) dest;
	BYTE* const oend = op + maxOutputSize;
	BYTE* cpy;

	size_t dec[] ={0, 3, 2, 3, 0, 0, 0, 0};

    static U16 u16_tmp0, u16_tmp1;
    static U32 u32_tmp0, u32_tmp1;

	// Main Loop
	while (ip<iend)
	{
		BYTE token;
		int length;

		// get runlength
		token = *ip++;
		if ((length=(token>>ML_BITS)) == RUN_MASK) { int s=255; while ((ip<iend) && (s==255)) { s=*ip++; length += s; } }

		// copy literals
		cpy = op+length;
		if ((cpy>oend-COPYLENGTH) || (ip+length>iend-COPYLENGTH))
		{
			if (cpy > oend) goto _output_error;          // Error : request to write beyond destination buffer
			if (ip+length > iend) goto _output_error;    // Error : request to read beyond source buffer
			memcpy(op, ip, length);
			op += length;
			ip += length;
			if (ip<iend) goto _output_error;             // Error : LZ4 format violation
			break;    // Necessarily EOF, due to parsing restrictions
		}

        do { 
            *op++ = *ip++;
            *op++ = *ip++;
            *op++ = *ip++;
            *op++ = *ip++;

            *op++ = *ip++;
            *op++ = *ip++;
            *op++ = *ip++;
            *op++ = *ip++;
        } while (op<cpy);

        ip -= (op-cpy); op = cpy;

		// get offset
		ref = cpy - (ip[0] | ip[1] << 8);
        ip+=2;
		if (ref < (BYTE* const)dest) goto _output_error;   // Error : offset creates reference outside of destination buffer

		// get matchlength
		if ((length=(token&ML_MASK)) == ML_MASK) { while (ip<iend) { int s = *ip++; length +=s; if (s==255) continue; break; } }

		// copy repeated sequence
		if (unlikely(op-ref<STEPSIZE))
		{
			const int dec2 = 0;

			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			ref -= dec[op-ref];

            op[0] = ref[0];
            op[1] = ref[1];
            op[2] = ref[2];
            op[3] = ref[3];
            op += STEPSIZE-4;
			ref -= dec2;
		} else { 
            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
        }
		cpy = op + length - (STEPSIZE-4);
		if (cpy>oend-COPYLENGTH)
		{
			if (cpy > oend) goto _output_error;           // Error : request to write outside of destination buffer
			do {
                *op++ = *ref++;
                *op++ = *ref++;
                *op++ = *ref++;
                *op++ = *ref++;

                *op++ = *ref++;
                *op++ = *ref++;
                *op++ = *ref++;
                *op++ = *ref++;
            } while (op<(oend-COPYLENGTH));

			while(op<cpy) *op++=*ref++;
			op=cpy;
			if (op == oend) break;    // Check EOF (should never happen, since last 5 bytes are supposed to be literals)
			continue;
		}

		do {
            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;

            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
        } while (op<cpy);
		op=cpy;		// correction
	}

	// end of decoding
	return (int) (((char*)op)-dest);

	// write overflow error detected
_output_error:
	return (int) (-(((char*)ip)-source));
}
//-[TCCQB]
//
