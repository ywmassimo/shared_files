#include "stdafx.h"

#include "questmanager.h"
#include "DragonLair.h"
#include "char.h"
#include "guild.h"

namespace quest
{
	ALUA(dl_startRaid)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		long baseMapIndex = lua_tonumber(L, -1);

		CDragonLairManager::instance().Start(ch->GetMapIndex(), baseMapIndex, ch->GetGuild()->GetID());

		return 0;
	}

	void RegisterDragonLairFunctionTable()
	{
		luaL_reg dl_functions[] =
		{
			{	"startRaid",	dl_startRaid	},

			{	NULL,			NULL			}
		};

		CQuestManager::instance(). AddLuaFunctionTable("DragonLair", dl_functions);
	}
}
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
