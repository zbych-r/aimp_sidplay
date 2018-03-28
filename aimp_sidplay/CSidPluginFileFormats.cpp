#include "stdafx.h"
#include "CSidPluginFileFormats.h"
#include "AimpStringHelper.h"

CSidPluginFileFormats::CSidPluginFileFormats(IAIMPCore* core): m_description(L"SID file"), m_extensionList(L"*.sid;")
{
	core->AddRef();
	m_core = core;
	AddRef();
}


CSidPluginFileFormats::~CSidPluginFileFormats()
{
	m_core->Release();
}


HRESULT CSidPluginFileFormats::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPExtensionFileFormat)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}


HRESULT CSidPluginFileFormats::GetDescription(IAIMPString ** S)
{
	*S = AimpStringHelper::Wstring2AIMPString(m_description, m_core); //(*m_description);
	return S_OK;
}

HRESULT CSidPluginFileFormats::GetExtList(IAIMPString ** S)
{
	*S = AimpStringHelper::Wstring2AIMPString(m_extensionList, m_core); //(*m_description);
	return S_OK;
}

HRESULT CSidPluginFileFormats::GetFlags(DWORD * S)
{
	*S = AIMP_SERVICE_FILEFORMATS_CATEGORY_AUDIO;
	return S_OK;
}
