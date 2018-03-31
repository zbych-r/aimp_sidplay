#include "stdafx.h"
#include "CSidFileInfoProvider.h"
#include "CSidplayPlugin.h"

CSidFileInfoProvider::CSidFileInfoProvider()
{
	AddRef();
}


CSidFileInfoProvider::~CSidFileInfoProvider()
{
}


HRESULT CSidFileInfoProvider::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPExtensionFileInfoProvider)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	return E_NOTIMPL;
}


HRESULT CSidFileInfoProvider::GetFileInfo(IAIMPString * FileURI, IAIMPFileInfo * Info)
{
	wstring fileUri = wstring(FileURI->GetData());
	
	if (fileUri.find_first_of(L".sid") == wstring::npos)
	{
		return E_FAIL;
	}
	CSidplayPlugin* pluginContext = CSidplayPlugin::Instance();
	if (pluginContext->FillFileInfo(FileURI, Info) == false)
	{
		return E_FAIL;
	}
	return S_OK;
}
