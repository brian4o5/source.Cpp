//---------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>
#include <assert.h>
#define Byte zlib_Byte
#include "zlib.h"
#undef Byte
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#endif
#define CHUNK 16384

#include <list>


#include "CZLib.h"
#include "util.h"

using namespace core::util;
//---------------------------------------------------------------------------
class my_zlib
{
private:
	HINSTANCE dll;
	int (*deflate)(z_stream_s *,int);
	int (*deflateInit_)(z_stream_s *,int, const char*,int);
	int (*deflateEnd)(z_stream_s *);
	int (*inflate)(z_stream_s *, int);
	int (*inflateInit_)(z_stream_s *, const char*,int);
	int (*inflateEnd)(z_stream_s *);
	int (*compress)(unsigned char *,unsigned long *,unsigned char *,unsigned long);
	int (*compressBound)(unsigned long);
public:
	my_zlib()
	{
#ifdef _WIN64
		dll = LoadLibrary(L"zlibwapi.dll");
		if(dll == NULL)
			throw( EZLibException("Could not find zlibwapi.dll in the PATH"));
#else
		dll = LoadLibrary(L"zlib1.dll");
		if(dll == NULL)
			throw( EZLibException("Could not find zlib1.dll in the PATH"));
#endif
		if(!(deflate = (int(*)(z_stream_s*,int)) GetProcAddress(dll, "deflate")))
			throw(EZLibException("dll error"));
		if(!(deflateInit_ = (int(*)(z_stream_s*,int, const char*,int)) GetProcAddress(dll, "deflateInit_")))
			throw(EZLibException("dll error"));
		if(!(deflateEnd = (int(*)(z_stream_s*)) GetProcAddress(dll, "deflateEnd")))
			throw(EZLibException("dll error"));
		if(!(inflate = (int(*)(z_stream_s*,int)) GetProcAddress(dll, "inflate")))
			throw(EZLibException("dll error"));
		if(!(inflateInit_ = (int(*)(z_stream_s*, const char*,int)) GetProcAddress(dll, "inflateInit_")))
			throw(EZLibException("dll error"));
		if(!(inflateEnd = (int(*)(z_stream_s*)) GetProcAddress(dll, "inflateEnd")))
			throw(EZLibException("dll error"));
		if(!(compress = (int(*)(unsigned char *,unsigned long *,unsigned char *,unsigned long)) GetProcAddress(dll, "compress")))
			throw(EZLibException("dll error"));
		if(!(compressBound = (int(*)(unsigned long)) GetProcAddress(dll, "compressBound")))
			throw(EZLibException("dll error"));
	}
	int Deflate(z_stream_s * s,int i)
	{
		return deflate(s,i);
	}
	int DeflateInit(z_stream_s * s,int i)
	{
		return deflateInit(s,i);
	}
	int DeflateEnd(z_stream_s * s)
	{
		return deflateEnd(s);
	}
	int Inflate(z_stream_s * s,int i)
	{
		return inflate(s,i);
	}
	int InflateInit(z_stream_s * s)
	{
		return inflateInit(s);
	}
	int InflateEnd(z_stream_s * s)
	{
		return inflateEnd(s);
	}
	int Compress( unsigned char * p1,unsigned long * p2,unsigned char * p3 ,unsigned long p4)
	{
		return compress(p1,p2,p3,p4);
	}
	int CompressBound( unsigned long l)
	{
        return compressBound( l );
    }
};

my_zlib * zlib=0;


CZLib::CZLib()
{

	if(zlib == 0)
		zlib = new my_zlib();
}

//---------------------------------------------------------------------------
/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int CZLib::ZipFile(std::wstring fileIn, std::wstring fileOut, int level)
{
	FILE * source = 0;
	fopen_s(&source,utf8(fileIn).c_str(), "rb");
	FILE * dest = 0;
	fopen_s(&dest,utf8(fileOut).c_str(), "wb");
	if(!source || !dest)
	{
		if(source)
			fclose(source);
		if(dest)
			fclose(dest);
		return 0;
	}
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = zlib->DeflateInit(&strm, level);
	if (ret != Z_OK)
		return ret;

	/* compress until end of file */
	do {
		strm.avail_in = (uInt)fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)zlib->DeflateEnd(&strm);
			return Z_ERRNO;
		}
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
			ret = zlib->Deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)zlib->DeflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
	(void)zlib->DeflateEnd(&strm);
    return Z_OK;
}
//---------------------------------------------------------------------------
/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int CZLib::UnzipFile(std::wstring fileIn, std::wstring fileOut)
{
	FILE * source = 0;
	_wfopen_s(&source, fileIn.c_str(), L"rb");
	FILE * dest = 0;
	_wfopen_s(&dest,fileOut.c_str(), L"wb");
	if(!source || !dest)
	{
		if(source)
			fclose(source);
		if(dest)
			fclose(dest);
		return 0;
	}
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	/* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = zlib->InflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = (uInt)fread(in, 1, CHUNK, source);
        if (ferror(source)) {
			(void)zlib->InflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
			ret = zlib->Inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
				(void)zlib->InflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)zlib->InflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)zlib->InflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
//---------------------------------------------------------------------------
/* report a zlib or i/o error */
std::wstring CZLib::ZErr(int ret)
{
	switch (ret) {
	case Z_ERRNO:
		return L"IO error";
	case Z_STREAM_ERROR:
		return L"invalid compression level";
	case Z_DATA_ERROR:
		return L"invalid or incomplete deflate data";
	case Z_MEM_ERROR:
		return L"out of memory";
	case Z_VERSION_ERROR:
		return L"zlib version mismatch!";
	case Z_BUF_ERROR:
    	return L"working buffer is too small";
	case Z_OK:
		return L"OK";
	}
	return L"";
}
//---------------------------------------------------------------------------
int CZLib::Compress(BYTE * inBytes, unsigned long inSize, BYTE ** outBytes, unsigned long & outSize)
{
	outSize = zlib->CompressBound(inSize);

	*outBytes = new BYTE[outSize];
	BYTE * out = *outBytes;
	if(out)
		return zlib->Compress((Bytef*)out,&outSize,(Bytef*)inBytes, inSize);
	else
		return Z_MEM_ERROR;
}
//---------------------------------------------------------------------------
int CZLib::Decompress(BYTE * inBytes, unsigned long inSize, BYTE * &outBytes, unsigned long & outSize)
{
	outSize = 0;
	BYTE * b = inBytes;
	uLong remaining = inSize;
	std::list<rec> done;

	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = zlib->InflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		if(remaining == 0)
			break;
		strm.avail_in = remaining < CHUNK ? remaining : CHUNK;
		remaining -= strm.avail_in;
		memcpy(in, b, strm.avail_in);
		b += strm.avail_in;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = zlib->Inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)zlib->InflateEnd(&strm);
				for(std::list<rec>::iterator itr=done.begin(); itr != done.end(); itr++)
					delete (*itr).data;
				return ret;
			}
			have = CHUNK - strm.avail_out;
			done.push_back(rec(new BYTE[have], have));
			memcpy(done.back().data,out,have);
			outSize+=have;
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)zlib->InflateEnd(&strm);

	outBytes = new BYTE[outSize+1]; // give room for a null at end for cleaness
	b = outBytes;
	if(!outBytes)
	{
		for(std::list<rec>::iterator itr=done.begin(); itr != done.end(); itr++)
			delete (*itr).data;
		return Z_MEM_ERROR;
	}

	for(std::list<rec>::iterator itr=done.begin(); itr != done.end(); itr++)
	{
		int size = (*itr).size;
		memcpy(b,(*itr).data,size);
		b+=size;
		delete (*itr).data;
	}
	outBytes[outSize]=0;
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

