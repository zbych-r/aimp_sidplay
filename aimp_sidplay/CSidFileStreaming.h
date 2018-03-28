#pragma once

#include "apiFileManager.h"
#include "apiCore.h"
#include "IUnknownInterfaceImpl.h"
#include "CSidFileExpander.h"
#include "CSidFileInfoProvider.h"

/// Calss responsible for creation of "expander" and "fileinfoprovider"
class CSidFileStreaming : public IUnknownInterfaceImpl<IAIMPServiceFileStreaming>
{
private:
	IAIMPCore * m_core;
	CSidFileExpander* m_sidFileExpander;
	CSidFileInfoProvider* m_sidFileInfoProvider;
public:
	CSidFileStreaming(IAIMPCore *core);
	~CSidFileStreaming();

	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual HRESULT WINAPI CreateStreamForFile(IAIMPString * FileName, DWORD Flags, const INT64 Offset, const INT64 Size, IAIMPStream ** Stream) override;
	virtual HRESULT WINAPI CreateStreamForFileURI(IAIMPString * FileURI, IAIMPVirtualFile ** VirtualFile, IAIMPStream ** Stream) override;
};

