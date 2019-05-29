//---------------------------------------------------------------------------

#pragma once
#include "libCore.h"
#include <exception>
#include <string>
//---------------------------------------------------------------------------

class EZLibException : public std::exception
{
public:
	EZLibException(const char * Msg) : exception(Msg) {}
};
class DLLEXPORT CZLib
{
struct rec
{
	rec(BYTE* d, int s) : data(d), size(s) {}
	BYTE * data;
	int size;
};
private:
public:
	CZLib();
	int UnzipFile(std::wstring in, std::wstring out);
	int ZipFile(std::wstring in, std::wstring out, int compressionLevel=6);
	std::wstring ZErr(int i);
	int Compress(BYTE * inBytes, unsigned long inSize, BYTE ** outBytes, unsigned long & outSize);
	int Decompress(BYTE * inBytes, unsigned long inSize, BYTE * &outBytes, unsigned long & outSize);
};

