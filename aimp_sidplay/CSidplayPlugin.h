#pragma once

//#include <corecrt_wstring.h>

#include "apiPlugin.h"
#include "IUnknownInterfaceImpl.h"
#include "CSidFileStreaming.h"
#include "CSidPluginFileFormats.h"
#include "CSidplayDecoderFactory.h"
#include "utils/SidDatabase.h"
#include "ThreadSidPlayer.h"

#include "resource.h"

using namespace std;

extern HINSTANCE moduleInstance;

class CSidplayPlugin : public IUnknownInterfaceImpl<IAIMPPlugin>, public IAIMPExternalSettingsDialog
{
private:
	static CSidplayPlugin* m_instance;

	CThreadSidPlayer* m_player;
	bool m_finalized;
	IAIMPCore* m_core;
	CSidFileStreaming* m_sidFileStreaming;
	CSidPluginFileFormats *m_fileFormats;
	CSidplayDecoderFactory* m_decoderFactory;
	SidDatabase m_sidDatabase;

	FILE* hf_out;

	void OpenConsole();
	CSidplayPlugin();
public:
	
	~CSidplayPlugin() {
		Finalize();
		m_instance = nullptr;
	}
	static CSidplayPlugin *Instance() {
		if (!m_instance)
			m_instance = new CSidplayPlugin();

		return m_instance;
	}

	bool FillFileInfo(IAIMPString * FileURI, IAIMPFileInfo * Info);
	bool FillFileInfo(const wchar_t* fileUri, IAIMPFileInfo * Info);
	bool FillFileInfo(wstring& fileUri, IAIMPFileInfo * Info);
	int GetTuneLength(SidTune& tune);
	CThreadSidPlayer* CreatePlayerInstance();
	void PrintToConsole(const wstring* str);

	// Inherited via IUnknownInterfaceImpl
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual PWCHAR WINAPI InfoGet(int Index) override;
	virtual DWORD WINAPI InfoGetCategories() override;
	virtual HRESULT WINAPI Initialize(IAIMPCore * Core) override;
	virtual HRESULT WINAPI Finalize() override;
	virtual void WINAPI SystemNotification(int NotifyID, IUnknown * Data) override;

	virtual ULONG WINAPI AddRef(void) override;
	virtual ULONG WINAPI Release(void) override;
	// Inherited via IAIMPExternalSettingsDialog
	virtual void WINAPI Show(HWND ParentWindow) override;

};

