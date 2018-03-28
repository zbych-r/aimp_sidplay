#pragma once

#include "apiDecoders.h"
#include "IUnknownInterfaceImpl.h"

class CSidplayDecoderFactory : public IUnknownInterfaceImpl<IAIMPExtensionAudioDecoder>
{
public:
	CSidplayDecoderFactory();
	~CSidplayDecoderFactory();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual HRESULT WINAPI CreateDecoder(IAIMPStream * Stream, DWORD Flags, IAIMPErrorInfo * ErrorInfo, IAIMPAudioDecoder ** Decoder) override;

};

