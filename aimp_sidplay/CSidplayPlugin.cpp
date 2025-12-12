#include "stdafx.h"

#include <string>
#include <io.h>
#include <fcntl.h>

#include "CSidplayPlugin.h"
#include "apiFileManager.h"
#include "sidplayfp/SidTuneInfo.h"
#include "sidplayfp/SidTune.h"
#include "AimpStringHelper.h"
#include "configDlg.h"

HINSTANCE moduleInstance;

HRESULT __declspec(dllexport) STDMETHODCALLTYPE AIMPPluginGetHeader(IAIMPPlugin **Header) {
	//gSidplayPlugin = CSidplayPlugin::Instance();//new PlugTest();
	//gSidplayPlugin->AddRef();
	CSidplayPlugin* plugin = CSidplayPlugin::Instance();
	plugin->AddRef();
	*Header = plugin;
	return S_OK;
}

CSidplayPlugin* CSidplayPlugin::m_instance = NULL;

CSidplayPlugin::CSidplayPlugin()
{
	m_finalized = false;
	m_core = NULL;

}

HRESULT CSidplayPlugin::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPExternalSettingsDialog)
	{
		*ppvObject = (IAIMPExternalSettingsDialog*)this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;

}

CThreadSidPlayer* CSidplayPlugin::CreatePlayerInstance()
{
	CThreadSidPlayer* sidPlayer = new CThreadSidPlayer(&m_sidDatabase);
	return sidPlayer;
}

bool CSidplayPlugin::FillFileInfo(IAIMPString * FileURI, IAIMPFileInfo * Info)
{
	return FillFileInfo(FileURI->GetData(), Info);
}

bool CSidplayPlugin::FillFileInfo(wstring& fileUri, IAIMPFileInfo * Info)
{
	return FillFileInfo(fileUri.c_str(), Info);
}

bool CSidplayPlugin::FillFileInfo(const wchar_t* fileUri, IAIMPFileInfo * Info)
{
	//const int MAX_PATH = 512;
	IAIMPString *astr;
	wstring strUri(fileUri);
	size_t extensionPos = strUri.rfind(L".sid");
	size_t subsongPos = strUri.find(L":", extensionPos);

	if (extensionPos == wstring::npos)
	{
		return false;
	}
	if ((extensionPos + 4 < strUri.length()) && (strUri.at(extensionPos + 4) != L':'))
	{
		return false;
	}

	wstring fileNameOnly;
	int subsongNumber = 0;
	if (subsongPos == wstring::npos)
	{
		fileNameOnly.assign(strUri);
	}
	else
	{
		fileNameOnly = strUri.substr(0, subsongPos);
		wstring subsongStr = strUri.substr(subsongPos + 1);
		subsongNumber = std::stoi(subsongStr);
	}

	char filePath[MAX_PATH];
	wcstombs(filePath, fileNameOnly.c_str(), MAX_PATH);
	SidTune sidtune(filePath);
	if (sidtune.getStatus() == false)
	{
		return false;
	}
	const SidTuneInfo* sidInfo = sidtune.getInfo();

	wchar_t tempWchr[512];

	const char* info1 = sidInfo->infoString(0);
	int len = strlen(info1);
	if (subsongNumber > 0)
	{
		wchar_t tempTitle[512];
		mbstowcs(tempTitle, info1, len + 1);

		swprintf_s(tempWchr, L"%s (Sub %d)", tempTitle, subsongNumber);
		sidtune.selectSong(subsongNumber);
	
		wstring track(std::to_wstring(subsongNumber));
		astr = AimpStringHelper::Wstring2AIMPString(track, m_core);
		Info->SetValueAsObject(AIMP_FILEINFO_PROPID_TRACKNUMBER, reinterpret_cast<IUnknown*>(astr));

	}
	else
	{
		sidtune.selectSong(sidInfo->startSong());
		mbstowcs(tempWchr, info1, len + 1);
	}

	astr = AimpStringHelper::Wchar2AIMPString(tempWchr, m_core);
	Info->SetValueAsObject(AIMP_FILEINFO_PROPID_TITLE, reinterpret_cast<IUnknown*>(astr));

	const char* info2Artist = sidInfo->infoString(1);
	len = strlen(info2Artist);
	mbstowcs(tempWchr, info2Artist, len + 1);
	astr = AimpStringHelper::Wchar2AIMPString(tempWchr, m_core);
	Info->SetValueAsObject(AIMP_FILEINFO_PROPID_ARTIST, reinterpret_cast<IUnknown*>(astr));

	const char* info3Publisher = sidInfo->infoString(2);
	len = strlen(info3Publisher);
	mbstowcs(tempWchr, info3Publisher, len + 1);
	astr = AimpStringHelper::Wchar2AIMPString(tempWchr, m_core);
	Info->SetValueAsObject(AIMP_FILEINFO_PROPID_PUBLISHER, reinterpret_cast<IUnknown*>(astr));

	swprintf_s(tempWchr, L"Load addres: $%x \r\nInit address: $%x\r\nPlay address: $%x\r\nSID 2 Address $%x\r\nSID 3 Address $%x", sidInfo->loadAddr(), sidInfo->initAddr(),
		sidInfo->playAddr(), sidInfo->sidChipBase(1), sidInfo->sidChipBase(2));
	astr = AimpStringHelper::Wchar2AIMPString(tempWchr, m_core);
	Info->SetValueAsObject(AIMP_FILEINFO_PROPID_COMMENT, reinterpret_cast<IUnknown*>(astr));

	double duration = (double)m_sidDatabase.length(sidtune) ;	//m_player->GetSongLength(sidtune);
	if (duration <= 0)
	{
		if (m_player != nullptr)
		{
			PlayerConfig cfg = m_player->GetCurrentConfig();
			duration = m_player->GetCurrentConfig().playLimitSec;
		}
		else
		{
			duration = 180;
		}
	}
	Info->SetValueAsFloat(AIMP_FILEINFO_PROPID_DURATION, duration);

	return true;
}

PWCHAR CSidplayPlugin::InfoGet(int Index)
{
	switch (Index)
	{
	case AIMP_PLUGIN_INFO_NAME:
		return (PWCHAR)L"AIMP Sidplay2";
	case AIMP_PLUGIN_INFO_AUTHOR:
		return (PWCHAR)L"Zbigniew Ross";
	case AIMP_PLUGIN_INFO_SHORT_DESCRIPTION:
		return (PWCHAR)L"Play C64 SID files *.sid";
	case AIMP_PLUGIN_INFO_FULL_DESCRIPTION:
		return (PWCHAR)L"Play C64 SID files *.sid";
	}
	return PWCHAR();
}

DWORD CSidplayPlugin::InfoGetCategories()
{
	return AIMP_PLUGIN_CATEGORY_DECODERS;
}

HRESULT CSidplayPlugin::Initialize(IAIMPCore * Core)
{
	HRESULT res;
#if CONSOLE_DEBUG
	OpenConsole();
#endif

	m_core = Core;
	m_core->AddRef();

	CThreadSidPlayer* player = new CThreadSidPlayer(NULL);
	player->Init();
	PlayerConfig pc = player->GetCurrentConfig();
	if ((pc.useSongLengthFile) && (pc.songLengthsFile != NULL))
	{
		bool openRes = m_sidDatabase.open(pc.songLengthsFile);
		if (!openRes)
		{
			MessageBoxA(NULL, "Error opening songlength database.\r\nDisable songlength databse or choose other file", "in_sidplay2", MB_OK);
		}
	}
	delete player;
	

	//register known extension
	
	m_fileFormats = new CSidPluginFileFormats(m_core);
	//m_fileFormats->AddRef();
	res = Core->RegisterExtension(IID_IAIMPServiceFileFormats, static_cast<IAIMPExtensionFileFormat*>(m_fileFormats));
	if (FAILED(res))
	{
		//TODO
		return S_FALSE;
	}

	m_sidFileStreaming = new CSidFileStreaming(m_core);
	//m_sidFileStreaming->AddRef();
	res = Core->RegisterExtension(IID_IAIMPServiceFileStreaming, static_cast<IAIMPServiceFileStreaming*>(m_sidFileStreaming));
	if (FAILED(res))
	{
		return S_FALSE;
	}

	m_decoderFactory = new CSidplayDecoderFactory();
	//m_decoderFactory->AddRef();
	res = Core->RegisterExtension(IID_IAIMPServiceAudioDecoders, static_cast<IAIMPExtensionAudioDecoder*>(m_decoderFactory));
	if (FAILED(res))
	{
		return S_FALSE;
	}


	return S_OK;
}

HRESULT CSidplayPlugin::Finalize()
{
	if (m_finalized == true)
	{
		return S_OK;
	}
	if (m_core != NULL)
	{
		m_core->Release();
		m_sidFileStreaming->Release();
		m_fileFormats->Release();
		m_decoderFactory->Release();
		m_finalized = true;
		delete m_player;
	}
	return S_OK;
}

void CSidplayPlugin::SystemNotification(int NotifyID, IUnknown * Data)
{
}

ULONG CSidplayPlugin::AddRef(void)
{
	return IUnknownInterfaceImpl<IAIMPPlugin>::AddRef();
}

ULONG CSidplayPlugin::Release(void)
{
	return IUnknownInterfaceImpl<IAIMPPlugin>::Release();
}

void CSidplayPlugin::Show(HWND ParentWindow)
{	
	DialogBoxParam(moduleInstance, MAKEINTRESOURCE(IDD_CONFIG_DLG), ParentWindow, &ConfigDlgWndProc, NULL);
}


int CSidplayPlugin::GetTuneLength(SidTune& tune)
{
	return m_sidDatabase.length(tune);	
}

void CSidplayPlugin::OpenConsole()
{
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
}

void CSidplayPlugin::PrintToConsole(const wstring* str)
{
	//fprintf(hf_out, text);
	fwprintf(hf_out, str->c_str());
	fprintf(hf_out,"\r\n");
}