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
	CSidplayPlugin* pluginContext = CSidplayPlugin::Instance();
	pluginContext->FillFileInfo(FileURI, Info);
	return S_OK;

}
