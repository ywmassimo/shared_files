#pragma once
#include "../UserInterface/Locale_inc.h"

wchar_t Arabic_ConvSymbol(wchar_t c);

bool Arabic_IsInSpace(wchar_t code);
bool Arabic_IsInSymbol(wchar_t code);
bool Arabic_IsInPresentation(wchar_t code);
bool Arabic_HasPresentation(wchar_t* codes, int last);
size_t Arabic_MakeShape(wchar_t* src, size_t srcLen, wchar_t* dst, size_t dstLen);
wchar_t Arabic_ConvEnglishModeSymbol(wchar_t code);

#if defined(__BL_CLIENT_LOCALE_STRING__)
#include <string>
std::wstring Arabic_MakeReverseHyperlink(wchar_t* dst, size_t size);
#endif
	
