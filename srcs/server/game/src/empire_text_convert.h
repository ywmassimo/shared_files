#ifndef __INC_EMPIRE_CHAT_CONVERT
#define __INC_EMPIRE_CHAT_CONVERT

bool LoadEmpireTextConvertTable(DWORD dwEmpireID, const char* c_szFileName);
void ConvertEmpireText(DWORD dwEmpireID, char* szText, size_t len, int iPct);

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
