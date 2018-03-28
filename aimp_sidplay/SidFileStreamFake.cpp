#include "stdafx.h"
#include "SidFileStreamFake.h"


SidFileStreamFake::SidFileStreamFake(const wchar_t* fileName, int startSubsong)
{
	m_fileName = new wchar_t[wcslen(fileName)+1];
	wcscpy(m_fileName, fileName);
	m_subsong = startSubsong;
	m_position = 0;
	AddRef();
}


SidFileStreamFake::~SidFileStreamFake()
{
	delete[] m_fileName;
}

HRESULT SidFileStreamFake::QueryInterface(REFIID riid, void ** ppvObject)
{
	if((riid == IID_IAIMPStream)|| (riid == IID_ISidFileStream))
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

INT64 SidFileStreamFake::GetSize()
{
	return 3;
}

HRESULT SidFileStreamFake::SetSize(const INT64 Value)
{
	return E_NOTIMPL;
}

INT64 SidFileStreamFake::GetPosition()
{
	return m_position;//INT64();
}

HRESULT SidFileStreamFake::Seek(const INT64 Offset, int Mode)
{
	if (Offset <= 3)
	{
		m_position = Offset;
	}
	return S_OK;
}

int SidFileStreamFake::Read(unsigned char * Buffer, unsigned int Count)
{
	//return 0;
	if (m_position >= 3)
	{
		return 0;
	}
	if (Count >= 0)
	{
		strcpy((char*)Buffer, "SID");
	}
	m_position = 4;
	return 3;
}

HRESULT SidFileStreamFake::Write(unsigned char * Buffer, unsigned int Count, unsigned int * Written)
{
	return E_NOTIMPL;
}

wchar_t* SidFileStreamFake::GetFileName()
{
	return m_fileName;
}

int SidFileStreamFake::GetSubsong()
{
	return m_subsong;
}
