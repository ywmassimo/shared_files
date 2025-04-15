#ifndef __INC_METIN2_GAME_LOCALE_H__
#define __INC_METIN2_GAME_LOCALE_H__

extern "C"
{
	void locale_init(const char *filename);
	const char *locale_find(const char *string);

	extern int g_iUseLocale;

#define LC_TEXT(str) locale_find(str)
};

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
