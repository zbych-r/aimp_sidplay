#include "stdafx.h"
#include "CSidplayDecoderFactory.h"
#include "ISidFileStream.h"
#include "CSidplayDecoder.h"

CSidplayDecoderFactory::CSidplayDecoderFactory()
{
	AddRef();
}


CSidplayDecoderFactory::~CSidplayDecoderFactory()
{
}


HRESULT CSidplayDecoderFactory::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPExtensionAudioDecoder)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT CSidplayDecoderFactory::CreateDecoder(IAIMPStream * Stream, DWORD Flags, IAIMPErrorInfo * ErrorInfo, IAIMPAudioDecoder ** Decoder)
{
	ISidFileStream* sfs;
	if (Stream->QueryInterface(IID_ISidFileStream, (void**)&sfs) == S_OK)
	{
		wchar_t* fn = sfs->GetFileName();
		int ss = sfs->GetSubsong();
		CSidplayDecoder* sidDecoder= new CSidplayDecoder(fn, ss);
		*Decoder = sidDecoder;
		return S_OK;
	}
	return E_FAIL;
}
