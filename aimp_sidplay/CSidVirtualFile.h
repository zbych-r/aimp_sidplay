#pragma once

#include <string>
#include "apiFileManager.h"
#include "apiCore.h"
#include "IUnknownInterfaceImpl.h"

using namespace std;

class CSidVirtualFile : IUnknownInterfaceImpl<IAIMPVirtualFile>
{

private:
	const wstring FILE_FORMAT;

	IAIMPCore * m_core;
	wstring m_fileName;
	wstring m_fileUri;
	int m_startSubsong;
public:
	CSidVirtualFile(IAIMPCore* core, IAIMPString * FileName, int startSubsong);
	~CSidVirtualFile();

	// Inherited via IAIMPVirtualFile
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;
	virtual void WINAPI BeginUpdate() override;
	virtual void WINAPI EndUpdate() override;
	virtual HRESULT WINAPI Reset() override;
	virtual HRESULT WINAPI GetValueAsFloat(int PropertyID, double * Value) override;
	virtual HRESULT WINAPI GetValueAsInt32(int PropertyID, int * Value) override;
	virtual HRESULT WINAPI GetValueAsInt64(int PropertyID, INT64 * Value) override;
	virtual HRESULT WINAPI GetValueAsObject(int PropertyID, REFIID IID, void ** Value) override;
	virtual HRESULT WINAPI SetValueAsFloat(int PropertyID, const double Value) override;
	virtual HRESULT WINAPI SetValueAsInt32(int PropertyID, int Value) override;
	virtual HRESULT WINAPI SetValueAsInt64(int PropertyID, const INT64 Value) override;
	virtual HRESULT WINAPI SetValueAsObject(int PropertyID, IUnknown * Value) override;
	virtual HRESULT WINAPI CreateStream(IAIMPStream ** Stream) override;
	virtual HRESULT WINAPI GetFileInfo(IAIMPFileInfo * Info) override;
	virtual HRESULT WINAPI IsExists() override;
	virtual HRESULT WINAPI IsInSameStream(IAIMPVirtualFile * VirtualFile) override;
	virtual HRESULT WINAPI Synchronize() override;

};

