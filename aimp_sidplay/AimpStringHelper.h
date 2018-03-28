#pragma once

#include <string>

#include "apiCore.h"

class AimpStringHelper
{

public:

	static IAIMPString* Wstring2AIMPString(const std::wstring &string, IAIMPCore* core) {
		IAIMPString* astr;
		if (SUCCEEDED(core->CreateObject(IID_IAIMPString, reinterpret_cast<void **>(&astr)))) {
			astr->SetData(const_cast<wchar_t *>(string.data()), string.size());
		}
		return astr;
	}

	static IAIMPString* Wchar2AIMPString(const wchar_t *string, IAIMPCore* core) {
		IAIMPString* astr;
		if (SUCCEEDED(core->CreateObject(IID_IAIMPString, reinterpret_cast<void **>(&astr)))) {
			astr->SetData(const_cast<wchar_t *>(string), wcslen(string));
		}
		return astr;
	}
};

