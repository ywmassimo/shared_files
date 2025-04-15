#include "stdafx.h"
#include "HB.h"
#include "Main.h"
#include "DBManager.h"

#include <memory>

PlayerHB::PlayerHB()
{
	m_iExpireTime = 3600; // 1 hour hotbackup default.
}

PlayerHB::~PlayerHB()
{
}

bool PlayerHB::Initialize()
{
	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SHOW CREATE TABLE player%s", GetTablePostfix());

	auto pMsg(CDBManager::instance().DirectQuery(szQuery));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	m_stCreateTableQuery = row[1];
	return true;
}

//

//
void PlayerHB::Put(DWORD id)
{
	auto it = m_map_data.find(id);

	if (it == m_map_data.end())
	{
		Query(id);
		m_map_data.emplace(id, get_dword_time());
		return;
	}

	if (time(0) - it->second > m_iExpireTime)
		Query(id);
}

//
//
bool PlayerHB::Query(DWORD id)
{
	time_t ct = time(0);
	struct tm curr_tm = *localtime(&ct);
	char szTableName[64];
	snprintf(szTableName, sizeof(szTableName), "hb_%02d%02d%02d%02d_player%s",
			curr_tm.tm_year - 100, curr_tm.tm_mon + 1, curr_tm.tm_mday, curr_tm.tm_hour, GetTablePostfix());

	char szQuery[4096];

	if (m_stTableName.compare(szTableName))
	{
		char szFind[32];
		snprintf(szFind, sizeof(szFind), "CREATE TABLE `player%s`", GetTablePostfix());
		int pos = m_stCreateTableQuery.find(szFind);

		if (pos < 0)
		{
			sys_err("cannot find %s ", szFind);
			return false;
		}

		snprintf(szQuery, sizeof(szQuery), "CREATE TABLE IF NOT EXISTS %s%s", szTableName, m_stCreateTableQuery.c_str() + strlen(szFind));
		auto pMsg(CDBManager::instance().DirectQuery(szQuery, SQL_HOTBACKUP));
		m_stTableName = szTableName;
	}

	snprintf(szQuery, sizeof(szQuery), "REPLACE INTO %s SELECT * FROM %splayer%s WHERE id=%u", m_stTableName.c_str(), GetPlayerDBName(), GetTablePostfix(), id);
	CDBManager::instance().AsyncQuery(szQuery, SQL_HOTBACKUP);
	return true;
}
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
