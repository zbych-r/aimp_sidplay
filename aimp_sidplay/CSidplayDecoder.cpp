#include "stdafx.h"

#include "CSidplayDecoder.h"
#include "c64roms.h"
#include "ThreadSidPlayer.h"

#define PLAYBACK_BIT_PRECISION 16

int CSidplayDecoder::sDecoderIdGen = 0;

CSidplayDecoder::CSidplayDecoder(wchar_t* fileName, int subsong) 
{
	AddRef();
	m_fileName.assign(fileName);

	m_player = CSidplayPlugin::Instance()->CreatePlayerInstance();
	m_player->Init();
	m_player->Stop();

	char asciPath[MAX_PATH];
	wcstombs(asciPath, fileName, MAX_PATH);
	m_player->LoadTune(asciPath, subsong);
	//m_player->PlaySubtune(subsong);
}


CSidplayDecoder::~CSidplayDecoder()
{
	delete m_player;
}

HRESULT CSidplayDecoder::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPAudioDecoder)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

BOOL CSidplayDecoder::GetFileInfo(IAIMPFileInfo * FileInfo)
{
	CSidplayPlugin::Instance()->FillFileInfo(m_fileName.c_str(), FileInfo);
	return TRUE;
}

BOOL CSidplayDecoder::GetStreamInfo(int * SampleRate, int * Channels, int * SampleFormat)
{
	

	*SampleRate = m_player->GetCurrentConfig().sidConfig.frequency;
	*Channels = m_player->GetCurrentConfig().sidConfig.playback;
	*SampleFormat = AIMP_DECODER_SAMPLEFORMAT_16BIT;
	return TRUE;
}

BOOL CSidplayDecoder::IsSeekable()
{
	return TRUE;
}

BOOL CSidplayDecoder::IsRealTimeStream()
{
	return FALSE;
}

INT64 CSidplayDecoder::GetAvailableData()
{
	return (m_player->GetTotalBytesCount() - m_player->GetDecodedBytesCount());
}

INT64 CSidplayDecoder::GetSize()
{
	return m_player->GetTotalBytesCount();
}

INT64 CSidplayDecoder::GetPosition()
{
	return m_player->GetDecodedBytesCount();
}

BOOL CSidplayDecoder::SetPosition(const INT64 Value)
{
	m_player->DoSeek(Value);
	return S_OK;
}

int CSidplayDecoder::Read(void * Buffer, int Count)
{

	
	return m_player->Decode(Buffer, Count);
}
