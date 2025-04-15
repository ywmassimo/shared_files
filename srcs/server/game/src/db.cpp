#include "stdafx.h"
#include <sstream>
#include "../../common/billing.h"
#include "../../common/length.h"

#include "db.h"

#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "p2p.h"
#include "log.h"
#include "login_data.h"
#include "locale_service.h"
#include "spam.h"
#include "shutdown_manager.h"

DBManager::DBManager() : m_bIsConnect(false)
{
}

DBManager::~DBManager()
{
}

bool DBManager::Connect(const char * host, const int port, const char * user, const char * pwd, const char * db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	if (!m_sql_direct.Setup(host, user, pwd, db, g_stLocale.c_str(), true, port))
		sys_err("cannot open direct sql connection to host %s", host);

	return m_bIsConnect;
}

void DBManager::Query(const char * c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	m_sql.AsyncQuery(szQuery);
}

std::unique_ptr<SQLMsg> DBManager::DirectQuery(const char * c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	return m_sql_direct.DirectQuery(szQuery);
}

bool DBManager::IsConnected()
{
	return m_bIsConnect;
}

void DBManager::ReturnQuery(int iType, DWORD dwIdent, void * pvData, const char * c_pszFormat, ...)
{
	//sys_log(0, "ReturnQuery %s", c_pszQuery);
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo * p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg * DBManager::PopResult()
{
	SQLMsg * p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void DBManager::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		if( NULL != pMsg->pvUserData )
		{
			switch( reinterpret_cast<CQueryInfo*>(pMsg->pvUserData)->iQueryType )
			{
				case QUERY_TYPE_RETURN:
					AnalyzeReturnQuery(pMsg);
					break;

				case QUERY_TYPE_FUNCTION:
					{
						CFuncQueryInfo* qi = reinterpret_cast<CFuncQueryInfo*>( pMsg->pvUserData );
						qi->f(pMsg);
						M2_DELETE(qi);
					}
					break;

				case QUERY_TYPE_AFTER_FUNCTION:
					{
						CFuncAfterQueryInfo* qi = reinterpret_cast<CFuncAfterQueryInfo*>( pMsg->pvUserData );
						qi->f();
						M2_DELETE(qi);
					}
					break;
			}
		}

		delete pMsg;
	}
}

CLoginData * DBManager::GetLoginData(DWORD dwKey)
{
	std::map<DWORD, CLoginData *>::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return NULL;

	return it->second;
}

void DBManager::InsertLoginData(CLoginData * pkLD)
{
	m_map_pkLoginData.emplace(pkLD->GetKey(), pkLD);
}

void DBManager::DeleteLoginData(CLoginData * pkLD)
{
	std::map<DWORD, CLoginData *>::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

	if (it == m_map_pkLoginData.end())
		return;

	sys_log(0, "DeleteLoginData %s %p", pkLD->GetLogin(), pkLD);

	mapLDBilling.erase(pkLD->GetLogin());

	M2_DELETE(it->second);
	m_map_pkLoginData.erase(it);
}

void DBManager::SetBilling(DWORD dwKey, bool bOn, bool bSkipPush)
{
	std::map<DWORD, CLoginData *>::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
	{
		sys_err("cannot find login key %u", dwKey);
		return;
	}

	CLoginData * ld = it->second;

	auto it2 = mapLDBilling.find(ld->GetLogin());

	if (it2 != mapLDBilling.end())
		if (it2->second != ld)
			DeleteLoginData(it2->second);

	mapLDBilling.emplace(ld->GetLogin(), ld);

	if (ld->IsBilling() && !bOn && !bSkipPush)
		PushBilling(ld);

	SendLoginPing(ld->GetLogin());
	ld->SetBilling(bOn);
}

void DBManager::PushBilling(CLoginData * pkLD)
{
	TUseTime t;

	t.dwUseSec = (get_dword_time() - pkLD->GetLogonTime()) / 1000;

	if (t.dwUseSec <= 0)
		return;

	pkLD->SetLogonTime();
	long lRemainSecs = pkLD->GetRemainSecs() - t.dwUseSec;
	pkLD->SetRemainSecs(MAX(0, lRemainSecs));

	t.dwLoginKey = pkLD->GetKey();
	t.bBillType = pkLD->GetBillType();

	sys_log(0, "BILLING: PUSH %s %u type %u", pkLD->GetLogin(), t.dwUseSec, t.bBillType);

	if (t.bBillType == BILLING_IP_FREE || t.bBillType == BILLING_IP_TIME || t.bBillType == BILLING_IP_DAY)
		snprintf(t.szLogin, sizeof(t.szLogin), "%u", pkLD->GetBillID());
	else
		strlcpy(t.szLogin, pkLD->GetLogin(), sizeof(t.szLogin));

	strlcpy(t.szIP, pkLD->GetIP(), sizeof(t.szIP));

	m_vec_kUseTime.emplace_back(t);
}

void DBManager::FlushBilling(bool bForce)
{
	if (bForce)
	{
		std::map<DWORD, CLoginData *>::iterator it = m_map_pkLoginData.begin();

		while (it != m_map_pkLoginData.end())
		{
			CLoginData * pkLD = (it++)->second;

			if (pkLD->IsBilling())
				PushBilling(pkLD);
		}
	}

	if (!m_vec_kUseTime.empty())
	{
		DWORD dwCount = 0;

		std::vector<TUseTime>::iterator it = m_vec_kUseTime.begin();

		while (it != m_vec_kUseTime.end())
		{
			TUseTime * p = &(*(it++));

			// DISABLE_OLD_BILLING_CODE
			if (!g_bBilling)
			{
				++dwCount;
				continue;
			}

			Query("INSERT GameTimeLog (login, type, logon_time, logout_time, use_time, ip, server) "
					"VALUES('%s', %u, DATE_SUB(NOW(), INTERVAL %u SECOND), NOW(), %u, '%s', '%s')",
					p->szLogin, p->bBillType, p->dwUseSec, p->dwUseSec, p->szIP, g_stHostname.c_str());
			// DISABLE_OLD_BILLING_CODE_END

			switch (p->bBillType)
			{
				case BILLING_FREE:
				case BILLING_IP_FREE:
					break;

				case BILLING_DAY:
					{
						if (!bForce)
						{
							TUseTime * pInfo = M2_NEW TUseTime;
							memcpy(pInfo, p, sizeof(TUseTime));
							ReturnQuery(QID_BILLING_CHECK, 0, pInfo,
									"SELECT UNIX_TIMESTAMP(LimitDt)-UNIX_TIMESTAMP(NOW()),LimitTime FROM GameTime WHERE UserID='%s'", p->szLogin);
						}
					}
					break;

				case BILLING_TIME:
					{
						Query("UPDATE GameTime SET LimitTime=LimitTime-%u WHERE UserID='%s'", p->dwUseSec, p->szLogin);

						if (!bForce)
						{
							TUseTime * pInfo = M2_NEW TUseTime;
							memcpy(pInfo, p, sizeof(TUseTime));
							ReturnQuery(QID_BILLING_CHECK, 0, pInfo,
									"SELECT UNIX_TIMESTAMP(LimitDt)-UNIX_TIMESTAMP(NOW()),LimitTime FROM GameTime WHERE UserID='%s'", p->szLogin);
						}
					}
					break;

				case BILLING_IP_DAY:
					{
						if (!bForce)
						{
							TUseTime * pInfo = M2_NEW TUseTime;
							memcpy(pInfo, p, sizeof(TUseTime));
							ReturnQuery(QID_BILLING_CHECK, 0, pInfo,
									"SELECT UNIX_TIMESTAMP(LimitDt)-UNIX_TIMESTAMP(NOW()),LimitTime FROM GameTimeIP WHERE ipid=%s", p->szLogin);
						}
					}
					break;

				case BILLING_IP_TIME:
					{
						Query("UPDATE GameTimeIP SET LimitTime=LimitTime-%u WHERE ipid=%s", p->dwUseSec, p->szLogin);

						if (!bForce)
						{
							TUseTime * pInfo = M2_NEW TUseTime;
							memcpy(pInfo, p, sizeof(TUseTime));
							ReturnQuery(QID_BILLING_CHECK, 0, pInfo,
									"SELECT UNIX_TIMESTAMP(LimitDt)-UNIX_TIMESTAMP(NOW()),LimitTime FROM GameTimeIP WHERE ipid=%s", p->szLogin);
						}
					}
					break;
			}

			if (!bForce && ++dwCount >= 1000)
				break;
		}

		if (dwCount < m_vec_kUseTime.size())
		{
			int nNewSize = m_vec_kUseTime.size() - dwCount;
			memcpy(&m_vec_kUseTime[0], &m_vec_kUseTime[dwCount], sizeof(TUseTime) * nNewSize);
			m_vec_kUseTime.resize(nNewSize);
		}
		else
			m_vec_kUseTime.clear();

		sys_log(0, "FLUSH_USE_TIME: count %u", dwCount);
	}

	if (m_vec_kUseTime.size() < 10240)
	{
		DWORD dwCurTime = get_dword_time();

		std::map<DWORD, CLoginData *>::iterator it = m_map_pkLoginData.begin();

		while (it != m_map_pkLoginData.end())
		{
			CLoginData * pkLD = (it++)->second;

			if (!pkLD->IsBilling())
				continue;

			switch (pkLD->GetBillType())
			{
				case BILLING_IP_FREE:
				case BILLING_FREE:
					break;

				case BILLING_IP_DAY:
				case BILLING_DAY:
				case BILLING_IP_TIME:
				case BILLING_TIME:
					if (pkLD->GetRemainSecs() < 0)
					{
						DWORD dwSecsConnected = (dwCurTime - pkLD->GetLogonTime()) / 1000;

						if (dwSecsConnected % 10 == 0)
							SendBillingExpire(pkLD->GetLogin(), BILLING_DAY, 0, pkLD);
					}
					else if (pkLD->GetRemainSecs() <= 600) // if remain seconds lower than 10 minutes
					{
						DWORD dwSecsConnected = (dwCurTime - pkLD->GetLogonTime()) / 1000;

						if (dwSecsConnected >= 60) // 60 second cycle
						{
							sys_log(0, "BILLING 1 %s remain %d connected secs %u",
									pkLD->GetLogin(), pkLD->GetRemainSecs(), dwSecsConnected);
							PushBilling(pkLD);
						}
					}
					else
					{
						DWORD dwSecsConnected = (dwCurTime - pkLD->GetLogonTime()) / 1000;

						if (dwSecsConnected > (DWORD) (pkLD->GetRemainSecs() - 600) || dwSecsConnected >= 600)
						{
							sys_log(0, "BILLING 2 %s remain %d connected secs %u",
									pkLD->GetLogin(), pkLD->GetRemainSecs(), dwSecsConnected);
							PushBilling(pkLD);
						}
					}
					break;
			}
		}
	}
}

void DBManager::CheckBilling()
{
	std::vector<DWORD> vec;
	vec.emplace_back(0);

	//sys_log(0, "CheckBilling: map size %d", m_map_pkLoginData.size());

	auto it = m_map_pkLoginData.begin();

	while (it != m_map_pkLoginData.end())
	{
		CLoginData * pkLD = (it++)->second;

		if (pkLD->IsBilling())
		{
			sys_log(0, "BILLING: CHECK %u", pkLD->GetKey());
			vec.emplace_back(pkLD->GetKey());
		}
	}

	vec[0] = vec.size() - 1;
	db_clientdesc->DBPacket(HEADER_GD_BILLING_CHECK, 0, &vec[0], sizeof(DWORD) * vec.size());
}

void DBManager::SendLoginPing(const char * c_pszLogin)
{
	TPacketGGLoginPing ptog;

	ptog.bHeader = HEADER_GG_LOGIN_PING;
	strlcpy(ptog.szLogin, c_pszLogin, sizeof(ptog.szLogin));

	if (!g_pkAuthMasterDesc)  // If I am master, broadcast to others
	{
		P2P_MANAGER::instance().Send(&ptog, sizeof(TPacketGGLoginPing));
	}
	else // If I am slave send login ping to master
	{
		g_pkAuthMasterDesc->Packet(&ptog, sizeof(TPacketGGLoginPing));
	}
}

void DBManager::SendAuthLogin(LPDESC d)
{
	const TAccountTable & r = d->GetAccountTable();

	CLoginData * pkLD = GetLoginData(d->GetLoginKey());

	if (!pkLD)
		return;

	TPacketGDAuthLogin ptod;
	ptod.dwID = r.id;

	trim_and_lower(r.login, ptod.szLogin, sizeof(ptod.szLogin));
	strlcpy(ptod.szSocialID, r.social_id, sizeof(ptod.szSocialID));
	ptod.dwLoginKey = d->GetLoginKey();
	ptod.bBillType = pkLD->GetBillType();
	ptod.dwBillID = pkLD->GetBillID();

	std::memcpy(ptod.iPremiumTimes, pkLD->GetPremiumPtr(), sizeof(ptod.iPremiumTimes));
	std::memcpy(&ptod.adwClientKey, pkLD->GetClientKey(), sizeof(DWORD) * 4);

	db_clientdesc->DBPacket(HEADER_GD_AUTH_LOGIN, d->GetHandle(), &ptod, sizeof(TPacketGDAuthLogin));
	sys_log(0, "SendAuthLogin %s key %u", ptod.szLogin, ptod.dwID);

	SendLoginPing(r.login);
}

void DBManager::LoginPrepare(BYTE bBillType, DWORD dwBillID, long lRemainSecs, LPDESC d, DWORD * pdwClientKey, int * paiPremiumTimes)
{
	const TAccountTable & r = d->GetAccountTable();

	CLoginData * pkLD = M2_NEW CLoginData;

	pkLD->SetKey(d->GetLoginKey());
	pkLD->SetLogin(r.login);
	pkLD->SetBillType(bBillType);
	pkLD->SetBillID(dwBillID);
	pkLD->SetRemainSecs(lRemainSecs);
	pkLD->SetIP(d->GetHostName());
	pkLD->SetClientKey(pdwClientKey);

	if (paiPremiumTimes)
		pkLD->SetPremium(paiPremiumTimes);

	InsertLoginData(pkLD);

	SendAuthLogin(d);
}

bool GetGameTimeIP(MYSQL_RES * pRes, BYTE & bBillType, DWORD & dwBillID, int & seconds, const char * c_pszIP)
{
	if (!pRes)
		return true;

	MYSQL_ROW row = mysql_fetch_row(pRes);
	int col = 0;

	str_to_number(dwBillID, row[col++]);

	int ip_start = 0;
	str_to_number(ip_start, row[col++]);

	int ip_end = 0;
	str_to_number(ip_end, row[col++]);

	int type = 0;
	str_to_number(type, row[col++]);

	str_to_number(seconds, row[col++]);

	int day_seconds = 0;
	str_to_number(day_seconds, row[col++]);

	char szIP[MAX_HOST_LENGTH + 1];
	strlcpy(szIP, c_pszIP, sizeof(szIP));

	char * p = strrchr(szIP, '.');
	++p;

	int ip_postfix = 0;
	str_to_number(ip_postfix, p);
	int valid_ip = false;

	if (ip_start <= ip_postfix && ip_end >= ip_postfix)
		valid_ip = true;

	bBillType = BILLING_NONE;

	if (valid_ip)
	{
		if (type == -1)
			return false;

		if (type == 0)
			bBillType = BILLING_IP_FREE;
		else if (day_seconds > 0)
		{
			bBillType = BILLING_IP_DAY;
			seconds = day_seconds;
		}
		else if (seconds > 0)
			bBillType = BILLING_IP_TIME;
	}

	return true;
}

bool GetGameTime(MYSQL_RES * pRes, BYTE & bBillType, int & seconds)
{
	if (!pRes)
		return true;

	MYSQL_ROW row = mysql_fetch_row(pRes);
	sys_log(1, "GetGameTime %p %p %p", row[0], row[1], row[2]);

	int type = 0;
	str_to_number(type, row[0]);
	str_to_number(seconds, row[1]);
	int day_seconds = 0;
	str_to_number(day_seconds, row[2]);
	bBillType = BILLING_NONE;

	if (type == -1)
		return false;
	else if (type == 0)
		bBillType = BILLING_FREE;
	else if (day_seconds > 0)
	{
		bBillType = BILLING_DAY;
		seconds = day_seconds;
	}
	else if (seconds > 0)
		bBillType = BILLING_TIME;

	if (!g_bBilling)
		bBillType = BILLING_FREE;

	return true;
}

void SendBillingExpire(const char * c_pszLogin, BYTE bBillType, int iSecs, CLoginData * pkLD)
{
	TPacketBillingExpire ptod;

	strlcpy(ptod.szLogin, c_pszLogin, sizeof(ptod.szLogin));
	ptod.bBillType = bBillType;
	ptod.dwRemainSeconds = MAX(0, iSecs);
	db_clientdesc->DBPacket(HEADER_GD_BILLING_EXPIRE, 0, &ptod, sizeof(TPacketBillingExpire));
	sys_log(0, "BILLING: EXPIRE %s type %d sec %d ptr %p", c_pszLogin, bBillType, iSecs, pkLD);
}

void DBManager::AnalyzeReturnQuery(SQLMsg * pMsg)
{
	CReturnQueryInfo * qi = (CReturnQueryInfo *) pMsg->pvUserData;

	switch (qi->iType)
	{
		case QID_AUTH_LOGIN:
			{
				TPacketCGLogin3 * pinfo = (TPacketCGLogin3 *) qi->pvData;
				LPDESC d = DESC_MANAGER::instance().FindByLoginKey(qi->dwIdent);

				if (!d)
				{
					M2_DELETE(pinfo);
					break;
				}

				d->SetLogin(pinfo->login);

				sys_log(0, "QID_AUTH_LOGIN: START %u %p", qi->dwIdent, get_pointer(d));

				if (pMsg->Get()->uiNumRows == 0)
				{
					sys_log(0, "   NOID");
					LoginFailure(d, "NOID");
					M2_DELETE(pinfo);
				}
				else
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
					int col = 0;

					// PASSWORD('%s'), password, securitycode, social_id, id, status
					char szEncrytPassword[45 + 1] = {0, };
					char szPassword[45 + 1] = {0, };
					char szSocialID[SOCIAL_ID_MAX_LEN + 1] = {0, };
					char szStatus[ACCOUNT_STATUS_MAX_LEN + 1] = {0, };
					DWORD dwID = 0;

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
					   	break;
					}

					strlcpy(szEncrytPassword, row[col++], sizeof(szEncrytPassword));

					if (!row[col])
					{
					   	sys_err("error column %d", col);
						M2_DELETE(pinfo);
					   	break;
				   	}

					strlcpy(szPassword, row[col++], sizeof(szPassword));

					col++; // skip securitycode

					if (!row[col])
				   	{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
				   	}

					strlcpy(szSocialID, row[col++], sizeof(szSocialID));

					if (!row[col])
				   	{
					   	sys_err("error column %d", col);
						M2_DELETE(pinfo);
					   	break;
				   	}

					str_to_number(dwID, row[col++]);

					if (!row[col])
					{
					   	sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
				   	}

					strlcpy(szStatus, row[col++], sizeof(szStatus));

					BYTE bNotAvail = 0;
					str_to_number(bNotAvail, row[col++]);

					int aiPremiumTimes[PREMIUM_MAX_NUM];
					memset(&aiPremiumTimes, 0, sizeof(aiPremiumTimes));

					char szCreateDate[256] = "00000000";

					{
						str_to_number(aiPremiumTimes[PREMIUM_EXP], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_ITEM], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_SAFEBOX], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_AUTOLOOT], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_FISH_MIND], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_MARRIAGE_FAST], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_GOLD], row[col++]);

						{
							long retValue = 0;
							str_to_number(retValue, row[col++]);

							time_t create_time = retValue;
							struct tm * tm1;
							tm1 = localtime(&create_time);
							strftime(szCreateDate, 255, "%Y%m%d", tm1);

							sys_log(0, "Create_Time %d %s", retValue, szCreateDate);
							sys_log(0, "Block Time %d ", strncmp(szCreateDate, g_stBlockDate.c_str(), 8));
						}
					}

					int nPasswordDiff = strcmp(szEncrytPassword, szPassword);

					if (nPasswordDiff)
					{
						LoginFailure(d, "WRONGPWD");
						sys_log(0, "   WRONGPWD");
						M2_DELETE(pinfo);
					}
					else if (bNotAvail)
					{
						LoginFailure(d, "NOTAVAIL");
						sys_log(0, "   NOTAVAIL");
						M2_DELETE(pinfo);
					}
					else if (DESC_MANAGER::instance().FindByLoginName(pinfo->login))
					{
						LoginFailure(d, "ALREADY");
						sys_log(0, "   ALREADY");
						M2_DELETE(pinfo);
					}
					else if(!CShutdownManager::Instance().CheckCorrectSocialID(szSocialID) && !test_server)
					{
						LoginFailure(d, "BADSCLID");
						sys_log(0, "   BADSCLID");
						M2_DELETE(pinfo);
					}
					else if(CShutdownManager::Instance().CheckShutdownAge(szSocialID) && CShutdownManager::Instance().CheckShutdownTime())
					{
						LoginFailure(d, "AGELIMIT");
						sys_log(0, "   AGELIMIT");
						M2_DELETE(pinfo);
					}
					else if (strcmp(szStatus, "OK"))
					{
						LoginFailure(d, szStatus);
						sys_log(0, "   STATUS: %s", szStatus);
						M2_DELETE(pinfo);
					}
					else
					{
						{
							if (strncmp(szCreateDate, g_stBlockDate.c_str(), 8) >= 0)
							{
								LoginFailure(d, "BLKLOGIN");
								sys_log(0, "   BLKLOGIN");
								M2_DELETE(pinfo);
								break;
							}

							char szQuery[1024];
							snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW() WHERE id=%u", dwID);
							DBManager::instance().Query(szQuery);
						}

						TAccountTable & r = d->GetAccountTable();

						r.id = dwID;
						trim_and_lower(pinfo->login, r.login, sizeof(r.login));
						strlcpy(r.passwd, pinfo->passwd, sizeof(r.passwd));
						strlcpy(r.social_id, szSocialID, sizeof(r.social_id));
						DESC_MANAGER::instance().ConnectAccount(r.login, d);

						if (!g_bBilling)
						{
							LoginPrepare(BILLING_FREE, 0, 0, d, pinfo->adwClientKey, aiPremiumTimes);
							//By SeMinZ
							M2_DELETE(pinfo);
							break;
						}

						sys_log(0, "QID_AUTH_LOGIN: SUCCESS %s", pinfo->login);
					}
				}
			}
			break;

		case QID_BILLING_GET_TIME:
			{
				TPacketCGLogin3 * pinfo = (TPacketCGLogin3 *) qi->pvData;
				LPDESC d = DESC_MANAGER::instance().FindByLoginKey(qi->dwIdent);

				sys_log(0, "QID_BILLING_GET_TIME: START ident %u d %p", qi->dwIdent, get_pointer(d));

				if (d)
				{
					if (pMsg->Get()->uiNumRows == 0)
					{
						if (g_bBilling)
							LoginFailure(d, "NOBILL");
						else
							LoginPrepare(BILLING_FREE, 0, 0, d, pinfo->adwClientKey);
					}
					else
					{
						int seconds = 0;
						BYTE bBillType = BILLING_NONE;

						if (!GetGameTime(pMsg->Get()->pSQLResult, bBillType, seconds))
						{
							sys_log(0, "QID_BILLING_GET_TIME: BLOCK");
							LoginFailure(d, "BLOCK");
						}
						else if (bBillType == BILLING_NONE)
						{
							LoginFailure(d, "NOBILL");
							sys_log(0, "QID_BILLING_GET_TIME: NO TIME");
						}
						else
						{
							LoginPrepare(bBillType, 0, seconds, d, pinfo->adwClientKey);
							sys_log(0, "QID_BILLING_GET_TIME: SUCCESS");
						}
					}
				}
				M2_DELETE(pinfo);
			}
			break;

		case QID_BILLING_CHECK:
			{
				TUseTime * pinfo = (TUseTime *) qi->pvData;
				int iRemainSecs = 0;

				CLoginData * pkLD = NULL;

				if (pMsg->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

					int iLimitDt = 0;
					str_to_number(iLimitDt, row[0]);

					int iLimitTime = 0;
					str_to_number(iLimitTime, row[1]);

					pkLD = GetLoginData(pinfo->dwLoginKey);

					if (pkLD)
					{
						switch (pkLD->GetBillType())
						{
							case BILLING_TIME:
								if (iLimitTime <= 600 && iLimitDt > 0)
								{
									iRemainSecs = iLimitDt;
									pkLD->SetBillType(BILLING_DAY);
									pinfo->bBillType = BILLING_DAY;
								}
								else
									iRemainSecs = iLimitTime;
								break;

							case BILLING_IP_TIME:
								if (iLimitTime <= 600 && iLimitDt > 0)
								{
									iRemainSecs = iLimitDt;
									pkLD->SetBillType(BILLING_IP_DAY);
									pinfo->bBillType = BILLING_IP_DAY;
								}
								else
									iRemainSecs = iLimitTime;
								break;

							case BILLING_DAY:
							case BILLING_IP_DAY:
								iRemainSecs = iLimitDt;
								break;
						}

						pkLD->SetRemainSecs(iRemainSecs);
					}
				}

				SendBillingExpire(pinfo->szLogin, pinfo->bBillType, MAX(0, iRemainSecs), pkLD);
				M2_DELETE(pinfo);
			}
			break;

		case QID_SAFEBOX_SIZE:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

				if (ch)
				{
					if (pMsg->Get()->uiNumRows > 0)
					{
						MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
						int	size = 0;
						str_to_number(size, row[0]);
						ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * size);
					}
				}
			}
			break;

			// BLOCK_CHAT
		case QID_BLOCK_CHAT_LIST:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(qi->dwIdent);

				if (ch == NULL)
					break;
				if (pMsg->Get()->uiNumRows)
				{
					MYSQL_ROW row;
					while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "%s %s sec", row[0], row[1]);
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "No one currently blocked.");
				}
			}
			break;
			// END_OF_BLOCK_CHAT

		default:
			sys_err("FATAL ERROR!!! Unhandled return query id %d", qi->iType);
			break;
	}

	M2_DELETE(qi);
}

void DBManager::SendMoneyLog(BYTE type, DWORD vnum, int gold)
{
	if (!gold)
		return;
	TPacketMoneyLog p;
	p.type = type;
	p.vnum = vnum;
	p.gold = gold;
	db_clientdesc->DBPacket(HEADER_GD_MONEY_LOG, 0, &p, sizeof(p));
}

void DBManager::StopAllBilling()
{
	for (auto it = m_map_pkLoginData.begin(); it != m_map_pkLoginData.end(); ++it)
	{
		SetBilling(it->first, false);
	}
}

size_t DBManager::EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize)
{
	return m_sql_direct.EscapeString(dst, dstSize, src, srcSize);
}

//
// Common SQL
//
AccountDB::AccountDB() :
	m_IsConnect(false)
{
}

bool AccountDB::IsConnected()
{
	return m_IsConnect;
}

bool AccountDB::Connect(const char * host, const int port, const char * user, const char * pwd, const char * db)
{
	m_IsConnect = m_sql_direct.Setup(host, user, pwd, db, "", true, port);

	if (false == m_IsConnect)
	{
		fprintf(stderr, "cannot open direct sql connection to host: %s user: %s db: %s\n", host, user, db);
		return false;
	}

	return m_IsConnect;
}

bool AccountDB::ConnectAsync(const char * host, const int port, const char * user, const char * pwd, const char * db, const char * locale)
{
	m_sql.Setup(host, user, pwd, db, locale, false, port);
	return true;
}

void AccountDB::SetLocale(const std::string & stLocale)
{
	m_sql_direct.SetLocale(stLocale);
	m_sql_direct.QueryLocaleSet();
}

std::unique_ptr<SQLMsg> AccountDB::DirectQuery(const char * query)
{
	return m_sql_direct.DirectQuery(query);
}

void AccountDB::AsyncQuery(const char* query)
{
	m_sql.AsyncQuery(query);
}

void AccountDB::ReturnQuery(int iType, DWORD dwIdent, void * pvData, const char * c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo * p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg * AccountDB::PopResult()
{
	SQLMsg * p;

	if (m_sql.PopResult(&p))
		return p;

	return NULL;
}

void AccountDB::Process()
{
	SQLMsg* pMsg = NULL;

	while ((pMsg = PopResult()))
	{
		CQueryInfo* qi = (CQueryInfo *) pMsg->pvUserData;

		switch (qi->iQueryType)
		{
			case QUERY_TYPE_RETURN:
				AnalyzeReturnQuery(pMsg);
				break;
		}
	}

	delete pMsg;
}

extern unsigned int g_uiSpamReloadCycle;

enum EAccountQID
{
	QID_SPAM_DB,
};

static LPEVENT s_pkReloadSpamEvent = NULL;

EVENTINFO(reload_spam_event_info)
{
	// used to send command
	DWORD empty;
};

EVENTFUNC(reload_spam_event)
{
	AccountDB::instance().ReturnQuery(QID_SPAM_DB, 0, NULL, "SELECT word, score FROM spam_db WHERE type='SPAM'");
	return PASSES_PER_SEC(g_uiSpamReloadCycle);
}

// #define ENABLE_SPAMDB_REFRESH
void LoadSpamDB()
{
	AccountDB::instance().ReturnQuery(QID_SPAM_DB, 0, NULL, "SELECT word, score FROM spam_db WHERE type='SPAM'");
#ifdef ENABLE_SPAMDB_REFRESH
	if (NULL == s_pkReloadSpamEvent)
	{
		reload_spam_event_info* info = AllocEventInfo<reload_spam_event_info>();
		s_pkReloadSpamEvent = event_create(reload_spam_event, info, PASSES_PER_SEC(g_uiSpamReloadCycle));
	}
#endif
}

void CancelReloadSpamEvent() {
	s_pkReloadSpamEvent = NULL;
}

void AccountDB::AnalyzeReturnQuery(SQLMsg * pMsg)
{
	CReturnQueryInfo * qi = (CReturnQueryInfo *) pMsg->pvUserData;

	switch (qi->iType)
	{
		case QID_SPAM_DB:
			{
				if (pMsg->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row;

					SpamManager::instance().Clear();

					while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
						SpamManager::instance().Insert(row[0], atoi(row[1]));
				}
			}
			break;
	}

	M2_DELETE(qi);
}
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
