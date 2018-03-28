#include "stdafx.h"
#include "CSidFileStreaming.h"
#include "CSidFileExpander.h"

CSidFileStreaming::CSidFileStreaming(IAIMPCore* core)
{
	AddRef();
	m_core = core;
	m_core->AddRef();
	m_sidFileExpander = new CSidFileExpander(m_core);
	m_sidFileInfoProvider = new CSidFileInfoProvider();
}


CSidFileStreaming::~CSidFileStreaming()
{
	m_core->Release();
	m_sidFileExpander->Release();
}


HRESULT CSidFileStreaming::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPServiceFileStreaming)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	
	if (riid == IID_IAIMPExtensionFileExpander)
	{
		m_sidFileExpander->AddRef();
		*ppvObject = m_sidFileExpander;
		return S_OK;
	}

	if (riid == IID_IAIMPExtensionFileInfoProvider)
	{
		m_sidFileInfoProvider->AddRef();
		*ppvObject = m_sidFileInfoProvider;
		return S_OK;
	}
	
	return E_NOINTERFACE;

}

HRESULT CSidFileStreaming::CreateStreamForFile(IAIMPString * FileName, DWORD Flags, const INT64 Offset, const INT64 Size, IAIMPStream ** Stream)
{
	return E_NOTIMPL;
}

HRESULT CSidFileStreaming::CreateStreamForFileURI(IAIMPString * FileURI, IAIMPVirtualFile ** VirtualFile, IAIMPStream ** Stream)
{
	return E_NOTIMPL;
}
