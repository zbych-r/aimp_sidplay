#include "stdafx.h"
#include "CSidFileExpander.h"
#include "CSidVirtualFile.h"

#include "sidplayfp/SidTuneInfo.h"
#include "sidplayfp/SidTune.h"
#include "utils/SidDatabase.h"

CSidFileExpander::CSidFileExpander(IAIMPCore* core)
{
	m_core = core;
	m_core->AddRef();
	AddRef();
}


CSidFileExpander::~CSidFileExpander()
{
	m_core->Release();
}

HRESULT CSidFileExpander::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (riid == IID_IAIMPExtensionFileExpander)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	return E_NOINTERFACE;
}


HRESULT CSidFileExpander::Expand(IAIMPString * FileName, IAIMPObjectList ** List, IAIMPProgressCallback * ProgressCallback)
{
	wstring filename = wstring(FileName->GetData());
	
	size_t extPos = filename.rfind(L".sid");
	if (extPos == wstring::npos)
	{
		return E_FAIL;
	}
	if(extPos + 4 < filename.length())
	{
		return E_FAIL;
	}

	if ((*List) == NULL)
	{
		m_core->CreateObject(IID_IAIMPObjectList, (void**)List);
		
		char fileName[MAX_PATH];
		wcstombs(fileName, FileName->GetData(), MAX_PATH);
		SidTune sidTune(fileName);
		if (sidTune.getStatus() == false)
		{
			return E_FAIL;
		}
		const SidTuneInfo *sidInfo = sidTune.getInfo();
		//NULL means usually that file doesn't exists
		if (sidInfo == NULL)
		{
			return E_FAIL;
		}

		CSidVirtualFile* vf;
		if (sidInfo->songs() > 1)
		{
			vf = new CSidVirtualFile(m_core, FileName, sidInfo->startSong());
			(*List)->Add(reinterpret_cast<IUnknown*>(vf));
			for (int i = 1; i <= sidInfo->songs(); ++i)
			{
				if (i == sidInfo->startSong())
				{
					continue;
				}
				vf = new CSidVirtualFile(m_core, FileName, i);
				(*List)->Add(reinterpret_cast<IUnknown*>(vf));
			}
		}
		else
		{
			vf = new CSidVirtualFile(m_core, FileName, 0);
			(*List)->Add(reinterpret_cast<IUnknown*>(vf));
		}
		return S_OK;
	}
	return E_FAIL;
}
