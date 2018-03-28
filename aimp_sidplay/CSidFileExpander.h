#pragma once

#include "IUnknownInterfaceImpl.h"
#include "apiFileManager.h"
#include "apiCore.h"


/// Class responsible for creation of songs and "subsong" entries
class CSidFileExpander : public IUnknownInterfaceImpl<IAIMPExtensionFileExpander>
{
private:
	IAIMPCore *m_core;
public:
	CSidFileExpander(IAIMPCore* core);
	~CSidFileExpander();

	// Inherited via IUnknownInterfaceImpl
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual HRESULT WINAPI Expand(IAIMPString * FileName, IAIMPObjectList ** List, IAIMPProgressCallback * ProgressCallback) override;
};

