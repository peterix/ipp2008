//---------------------------------------------------------------------------
//
// Name:        md5.cpp
// Author:      Kai Krahn
// Created:     11.11.07 12:35

// Description: wxWidgets md5 implementation
//
// This code implements the MD5 message-digest algorithm.
// The algorithm is due to Ron Rivest. This code is based on the code
// written by Colin Plumb in 1993, no copyright is claimed.
// This code is in the public domain; do with it what you wish.
//
// This version implements the MD5 algorithm for the free GUI toolkit wxWidgets.
// Basic functionality, like MD5 hash creation out of strings and files, or
// to verify strings and files against a given MD5 hash, was added
// by Kai Krahn.
//
// This code is provided "as is" and comes without any warranty!
//---------------------------------------------------------------------------

#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>

#include <memory.h>		 /* for memcpy() */
#include "md5.h"

#ifdef sgi
#define HIGHFIRST
#endif

#ifdef sun
#define HIGHFIRST
#endif

#ifndef HIGHFIRST
#ifdef wxBYTE_ORDER
    #if wxBYTE_ORDER == wxBIG_ENDIAN
        #define HIGHFIRST
    #endif
#endif
#endif

//---------------------------------------------------------------------------
#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
/**
 * Note: this code is harmless on little-endian machines.
 */
void byteReverse(buf, longs)
    unsigned char *buf; unsigned longs;
{
    uint32 t;
    do {
	t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
	    ((unsigned) buf[1] << 8 | buf[0]);
	*(uint32 *) buf = t;
	buf += 4;
    } while (--longs);
}
#endif
//---------------------------------------------------------------------------

/**
    The one and only constructor
*/
MD5Context::MD5Context()
{
    this->MD5Init();   
}


/*!
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Context::MD5Init()
{
    this->buf[0] = 0x67452301;
    this->buf[1] = 0xefcdab89;
    this->buf[2] = 0x98badcfe;
    this->buf[3] = 0x10325476;

    this->bits[0] = 0;
    this->bits[1] = 0;
}


/*!
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 * @param buf some data to be added
 * @param len size of data
 */
void MD5Context::MD5Update(unsigned char *buf, unsigned len)
{
    uint32 t;

    /* Update bitcount */

    t = this->bits[0];
    if ((this->bits[0] = t + ((uint32) len << 3)) < t)
	this->bits[1]++; 	/* Carry from low to high */
    this->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) this->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	byteReverse(this->in, 16);
	this->MD5Transform((uint32 *) this->in); 
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy(this->in, buf, 64);
	byteReverse(this->in, 16);
	this->MD5Transform((uint32 *) this->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(this->in, buf, len);
}


/*!
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 * @param digest contains the MD5 hash
 */
void MD5Context::MD5Final(unsigned char digest[16])
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (this->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = this->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, count);
	byteReverse(this->in, 16);
	this->MD5Transform((uint32 *) this->in);

	/* Now fill the next block with 56 bytes */
	memset(this->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(this->in, 14);

    /* Append length in bits and transform */
    ((uint32 *) this->in)[14] = this->bits[0];
    ((uint32 *) this->in)[15] = this->bits[1];

    this->MD5Transform((uint32 *) this->in);
    byteReverse((unsigned char *) this->buf, 4);
    memcpy(digest, this->buf, 16);
    //memset(ctx, 0, sizeof(ctx));        /* In case it's sensitive */
}


/** The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/** This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )


/*!
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Context::MD5Transform(uint32 in[16])
{
    register uint32 a, b, c, d;

    a = this->buf[0];
    b = this->buf[1];
    c = this->buf[2];
    d = this->buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    this->buf[0] += a;
    this->buf[1] += b;
    this->buf[2] += c;
    this->buf[3] += d;
}

//---------------------------------------------------------------------------

/*!
 * wxWidgets implementation
 * Creates the MD5 hash out of a string
 */
wxString MD5(wxString input)
{
	MD5Context* ctx = new MD5Context();

	unsigned int length = input.Len();
    unsigned char* Buffer = new unsigned char[length];

    wxStringInputStream* InStream = new wxStringInputStream(input);
    InStream->Read(Buffer, length);

    ctx->MD5Update(Buffer, length);

    unsigned char MD5Hash[16];
    ctx->MD5Final(MD5Hash);

    wxString out = wxEmptyString;
    for(int i=0; i<16;i++)
         out += wxString::Format(wxT("%02X"), MD5Hash[i]);

    delete [] Buffer;
    delete InStream;
    delete ctx;

    return out;
}


wxString MD5(unsigned char* Data, unsigned int Len)
{
	MD5Context* ctx = new MD5Context();

    ctx->MD5Update(Data, Len);

    unsigned char MD5Hash[16];
    ctx->MD5Final(MD5Hash);

    wxString out = wxEmptyString;
    for(int i=0; i<16;i++)
         out += wxString::Format(wxT("%02X"), MD5Hash[i]);

    delete ctx;
    
    return out;
}


wxString MD5(void* Data, unsigned int Len)
{
    unsigned char* buffer = new unsigned char[Len];
    memcpy(buffer, Data, Len);

    wxString value =  MD5(buffer, Len);
    delete [] buffer;

    return value;
}


wxString FileMD5(wxString FileName)
{
	MD5Context* ctx = new MD5Context();

    size_t ChunkSize = 1024;
    wxString out = wxEmptyString;

    if(wxFileName::FileExists(FileName))
        {
            wxFileInputStream* InStream = new wxFileInputStream(FileName);
            size_t FileSize = InStream->GetSize();

            size_t BytesRead = 0;
            while(BytesRead < FileSize)
                {
                    size_t BytesLeft = FileSize - BytesRead;
                    size_t count = wxMin(ChunkSize, BytesLeft);

                    unsigned char* Buffer = new unsigned char[count];
                    InStream->Read(Buffer, count);
                    BytesRead += count;

                    ctx->MD5Update(Buffer, count);
                    delete [] Buffer;
                }

            delete InStream;

            unsigned char MD5Hash[16];
            ctx->MD5Final(MD5Hash);

            for(int i=0; i<16;i++)
               out += wxString::Format(wxT("%02X"), MD5Hash[i]);
        }

    delete ctx;
    
    return out;
}


bool VerifyMD5(wxString MD5Hash, wxString input)
{
    wxString temp = MD5(input);
    return temp.IsSameAs(MD5Hash, false);
}


bool VerifyFileMD5(wxString MD5Hash, wxString FileName)
{
    wxString temp = FileMD5(FileName);
    return temp.IsSameAs(MD5Hash, false);
}
