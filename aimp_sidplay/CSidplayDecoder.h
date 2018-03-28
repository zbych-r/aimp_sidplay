#pragma once

#include <string>

#include "apiDecoders.h"
#include "IUnknownInterfaceImpl.h"
#include "typesdefs.h"
#include "CSidplayPlugin.h"

#include "sidplayfp/SidTune.h"
#include "sidplayfp/SidTuneInfo.h"
#include "utils/SidDatabase.h"
#include "SidInfoImpl.h"

#include "residfp.h"
#include "sidplayfp/sidplayfp.h"
#include "StilBlock.h"

using namespace std;

class CSidplayDecoder : public IUnknownInterfaceImpl<IAIMPAudioDecoder>
{
private:
	static int sDecoderIdGen;
private:
	wstring m_fileName;
	CThreadSidPlayer* m_player;
public:
	
	CSidplayDecoder(wchar_t* fileName, int subsong);
	~CSidplayDecoder();

	// Inherited via IUnknownInterfaceImpl
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual BOOL WINAPI GetFileInfo(IAIMPFileInfo * FileInfo) override;
	virtual BOOL WINAPI GetStreamInfo(int * SampleRate, int * Channels, int * SampleFormat) override;
	virtual BOOL WINAPI IsSeekable() override;
	virtual BOOL WINAPI IsRealTimeStream() override;
	virtual INT64 WINAPI GetAvailableData() override;
	virtual INT64 WINAPI GetSize() override;
	virtual INT64 WINAPI GetPosition() override;
	virtual BOOL WINAPI SetPosition(const INT64 Value) override;
	virtual int WINAPI Read(void * Buffer, int Count) override;
};

