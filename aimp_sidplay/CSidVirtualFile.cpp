#include "stdafx.h"
#include "CSidVirtualFile.h"
#include "SidFileStreamFake.h"
#include "CSidplayPlugin.h"
#include "AimpStringHelper.h"



CSidVirtualFile::CSidVirtualFile(IAIMPCore* core, IAIMPString * FileName, int startSubsong):FILE_FORMAT(L"SID")
{
	core->AddRef();
	m_core = core;

	m_fileName.assign(FileName->GetData());

	int bufferSize = wcslen(FileName->GetData()) + 5;
	wchar_t *wcUri = new wchar_t[bufferSize]; //+4 bo znak ":" , 3 cyfry i znak koñca
	swprintf_s(wcUri, bufferSize, L"%s:%d", FileName->GetData(), startSubsong);

	m_fileUri.assign(wcUri);	
	delete[] wcUri;
	m_startSubsong = startSubsong;	
	AddRef();
}


CSidVirtualFile::~CSidVirtualFile()
{
	m_core->Release();
}

HRESULT CSidVirtualFile::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPVirtualFile)
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}


void CSidVirtualFile::BeginUpdate()
{
}

void CSidVirtualFile::EndUpdate()
{
}

HRESULT CSidVirtualFile::Reset()
{
	return S_OK;
}

HRESULT CSidVirtualFile::GetValueAsFloat(int PropertyID, double * Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::GetValueAsInt32(int PropertyID, int * Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::GetValueAsInt64(int PropertyID, INT64 * Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::GetValueAsObject(int PropertyID, REFIID IID, void ** Value)
{
	if (IID == IID_IAIMPString)
	{
		switch (PropertyID)
		{
		case AIMP_VIRTUALFILE_PROPID_FILEURI:			
			*Value = AimpStringHelper::Wstring2AIMPString(m_fileUri, m_core); //AIMPString(m_fileUri);//(*m_fileUri);
			break;
		case AIMP_VIRTUALFILE_PROPID_AUDIOSOURCEFILE:
			
			*Value = AimpStringHelper::Wstring2AIMPString(m_fileName, m_core); //*AIMPString(m_fileName); //new AIMPString(m_fileName.c_str());
			break;
		case AIMP_VIRTUALFILE_PROPID_FILEFORMAT:
			*Value = AimpStringHelper::Wstring2AIMPString(FILE_FORMAT, m_core);
			break;
		}

	}
	return S_OK;
}

HRESULT CSidVirtualFile::SetValueAsFloat(int PropertyID, const double Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::SetValueAsInt32(int PropertyID, int Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::SetValueAsInt64(int PropertyID, const INT64 Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::SetValueAsObject(int PropertyID, IUnknown * Value)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::CreateStream(IAIMPStream ** Stream)
{
	
	SidFileStreamFake *sfs = new SidFileStreamFake(m_fileName.c_str(), m_startSubsong);
	//sfs->AddRef();
	*Stream = sfs;
	return S_OK;
}

HRESULT CSidVirtualFile::GetFileInfo(IAIMPFileInfo * Info)
{
	CSidplayPlugin* plugin = CSidplayPlugin::Instance();
	
	if (plugin->FillFileInfo(m_fileUri, Info) == false)
	{
		return E_FAIL;
	}
	return S_OK;
}

/// Sprawdza czy podane Ÿród³o plik istnieje
HRESULT CSidVirtualFile::IsExists()
{
	//return S_OK;
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::IsInSameStream(IAIMPVirtualFile * VirtualFile)
{
	return E_NOTIMPL;
}

HRESULT CSidVirtualFile::Synchronize()
{
	return S_OK;
}
