#pragma once

#include "apiFileManager.h"
// {F3C6C6F6-9647-4AF4-8C4D-627ECF3B09E6}
static const GUID IID_ISidFileStream ={ 0xf3c6c6f6, 0x9647, 0x4af4,{ 0x8c, 0x4d, 0x62, 0x7e, 0xcf, 0x3b, 0x9, 0xe6 } };

//static const GUID IID_IAIMPCore = { 0x41494D50, 0x436F, 0x7265, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


class ISidFileStream : public IAIMPStream
{
public:
	virtual wchar_t* WINAPI GetFileName() = 0;
	virtual int WINAPI GetSubsong() = 0;
};
