#pragma once

#include "apiCore.h"
#include "IUnknownInterfaceImpl.h"
#include "apiFileManager.h"
#include <string>

class CSidFileInfoProvider : public IUnknownInterfaceImpl<IAIMPExtensionFileInfoProvider>
{
public:
	CSidFileInfoProvider();
	~CSidFileInfoProvider();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual HRESULT WINAPI GetFileInfo(IAIMPString * FileURI, IAIMPFileInfo * Info) override;

};

