#pragma once

#include "ISidFileStream.h"
#include "IUnknownInterfaceImpl.h"

class SidFileStreamFake : public IUnknownInterfaceImpl<ISidFileStream>
{
private:
	int m_subsong;
	int m_position;
	wchar_t* m_fileName;
public:
	SidFileStreamFake(const wchar_t* fileName, int startSubsong );
	~SidFileStreamFake();

	// Inherited via IUnknownInterfaceImpl
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual INT64 WINAPI GetSize() override;
	virtual HRESULT WINAPI SetSize(const INT64 Value) override;
	virtual INT64 WINAPI GetPosition() override;
	virtual HRESULT WINAPI Seek(const INT64 Offset, int Mode) override;
	virtual int WINAPI Read(unsigned char * Buffer, unsigned int Count) override;
	virtual HRESULT WINAPI Write(unsigned char * Buffer, unsigned int Count, unsigned int * Written) override;
	virtual wchar_t* WINAPI GetFileName() override;
	virtual int WINAPI GetSubsong() override;
};

