#pragma once

#include <string>
#include "IUnknownInterfaceImpl.h"
#include "apiFileManager.h"
#include "apiCore.h"

using namespace std;

class CSidPluginFileFormats : public IUnknownInterfaceImpl<IAIMPExtensionFileFormat>
{
private:
	wstring m_description;
	wstring m_extensionList;
	IAIMPCore *m_core;
public:
	CSidPluginFileFormats(IAIMPCore* core);
	~CSidPluginFileFormats();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual HRESULT WINAPI GetDescription(IAIMPString ** S) override;
	virtual HRESULT WINAPI GetExtList(IAIMPString ** S) override;
	virtual HRESULT WINAPI GetFlags(DWORD * S) override;
};

