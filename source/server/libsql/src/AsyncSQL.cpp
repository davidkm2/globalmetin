#include "../include/AsyncSQL.h"
#include <errmsg.h>

#include <cstdlib>
#include <cstring>

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <cstdlib>
#include <cstring>



namespace
{

	bool ShouldRetry(uint32_t errorCode)
	{
		switch (errorCode) {
		case CR_SOCKET_CREATE_ERROR:
		case CR_CONNECTION_ERROR:
		case CR_IPSOCK_ERROR:
		case CR_UNKNOWN_HOST:
		case CR_SERVER_GONE_ERROR:
		case CR_CONN_HOST_ERROR:
		case CR_SERVER_LOST:
		case ER_NOT_KEYFILE:
		case ER_CRASHED_ON_USAGE:
		case ER_CANT_OPEN_FILE:
		case ER_HOST_NOT_PRIVILEGED:
		case ER_HOST_IS_BLOCKED:
		case ER_PASSWORD_NOT_ALLOWED:
		case ER_PASSWORD_NO_MATCH:
		case ER_CANT_CREATE_THREAD:
		case ER_INVALID_USE_OF_NULL:
			return true;

		default:
			return false;
		}
	}

}

SQLResult::SQLResult()
	: pSQLResult(nullptr)
	, uiNumRows(0)
	, uiAffectedRows(0)
	, uiInsertID(0)
{
}

SQLResult::~SQLResult()
{
	if (pSQLResult) {
		mysql_free_result(pSQLResult);
		pSQLResult = nullptr;
	}
}

SQLMsg::SQLMsg()
	: m_pkSQL(nullptr)
	, iID(0)
	, uiResultPos(0)
	, pvUserData(nullptr)
	, bReturn(false)
	, uiSQLErrno(0)
{
}

SQLMsg::~SQLMsg()
{
	for (const auto& p : vec_pkResult)
		delete p;
}

void SQLMsg::Store()
{
	do {
		auto pRes = new SQLResult;
		pRes->pSQLResult = mysql_store_result(m_pkSQL);
		pRes->uiInsertID = static_cast<uint32_t>(mysql_insert_id(m_pkSQL));
		pRes->uiAffectedRows = static_cast<uint32_t>(mysql_affected_rows(m_pkSQL));

		if (pRes->pSQLResult)
			pRes->uiNumRows = static_cast<uint32_t>(mysql_num_rows(pRes->pSQLResult));
		else
			pRes->uiNumRows = 0;

		vec_pkResult.push_back(pRes);
	} while (!mysql_next_result(m_pkSQL));
}

SQLResult* SQLMsg::Get()
{
	if (uiResultPos >= vec_pkResult.size())
		return nullptr;

	return vec_pkResult[uiResultPos];
}

bool SQLMsg::Next()
{
	if (uiResultPos + 1 >= vec_pkResult.size())
		return false;

	++uiResultPos;
	return true;
}

CAsyncSQL::CAsyncSQL()
	: m_connected(false)
	, m_iMsgCount(0)
	, m_iPort(0)
	, m_bEnd(false)
	, m_iQueryFinished(0)
{
	if (0 == mysql_init(&m_hDB))
		sys_err( "mysql_init failed");
}

CAsyncSQL::~CAsyncSQL()
{
	Quit();

	if (!m_stHost.empty()) {
		sys_log(0, "Disconnecting from %s@%s:%s",
			m_stUser.c_str(), m_stHost.c_str(), m_stDB.c_str());
	}

	mysql_close(&m_hDB);
}

bool CAsyncSQL::Connect()
{
    bool mysql_connection = false;

#ifndef _WIN32
	if (m_stHost.find("SOCKET") != std::string::npos)
    {
		sys_err("USING SOCKET CONNECTION on /tmp/mysql.sock!");
		mysql_connection = mysql_real_connect(&m_hDB, NULL, m_stUser.c_str(), m_stPassword.c_str(), m_stDB.c_str(), m_iPort, "/tmp/mysql.sock", CLIENT_MULTI_STATEMENTS);
	}
	else
#endif
    {
		mysql_connection = mysql_real_connect(&m_hDB, m_stHost.c_str(), m_stUser.c_str(), m_stPassword.c_str(), m_stDB.c_str(), m_iPort, NULL, CLIENT_MULTI_STATEMENTS);
    }

	if (!mysql_connection)
	{
		sys_err( "mysql_real_connect to %s@%s:%s: %s", m_stUser.c_str(), m_stHost.c_str(), m_stDB.c_str(), mysql_error(&m_hDB));
		return false;
	}

	my_bool reconnect = true;
	if (0 != mysql_options(&m_hDB, MYSQL_OPT_RECONNECT, &reconnect))
	{
		sys_err("mysql_option: %s", mysql_error(&m_hDB));
		return false;
	}

    // MYSQL ADJUSTEMENTS
	mysql_options(&m_hDB, MYSQL_OPT_COMPRESS, 0); // Use a compressed DB Connection

	if (mysql_set_character_set(&m_hDB, m_stLocale.c_str()))
	{
		sys_err("Failed to set locale %s with errno %u %s", m_stLocale.c_str(), mysql_errno(&m_hDB), mysql_error(&m_hDB));
	}

	sys_log(0, "Connected to MySQL server %s:%s as %s", m_stHost.c_str(), m_stDB.c_str(), m_stUser.c_str());
	m_connected = true;
	return true;
}

void CAsyncSQL::Quit()
{
	m_bEnd = true;
	m_sem.Release();

	if (m_asyncThread.joinable())
		m_asyncThread.join();
}

bool CAsyncSQL::Setup(CAsyncSQL* sql, bool bNoThread)
{
	return Setup(sql->m_stHost.c_str(),
		sql->m_stUser.c_str(),
		sql->m_stPassword.c_str(),
		sql->m_stDB.c_str(),
		sql->m_stLocale.c_str(),
		bNoThread,
		sql->m_iPort);
}

bool CAsyncSQL::Setup(const char* c_pszHost,
	const char* c_pszUser, const char* c_pszPassword,
	const char* c_pszDB, const char* c_pszLocale,
	bool bNoThread, int32_t iPort)
{
	m_stHost = c_pszHost;
	m_stUser = c_pszUser;
	m_stPassword = c_pszPassword;
	m_stDB = c_pszDB;
	m_iPort = iPort;

	if (c_pszLocale)
		m_stLocale = c_pszLocale;

	if (!bNoThread) {
		auto f = [this]() {
			mysql_thread_init();

			if (Connect())
				ChildLoop();

			mysql_thread_end();
		};

		assert(!m_asyncThread.joinable() &&
			"Thread must not be joinable, otherwise terminate() is called");

		std::thread thr(f);
		std::swap(m_asyncThread, thr);

		return true;
	}
	else {
		return Connect();
	}
}

void CAsyncSQL::SetLocale(const std::string& stLocale)
{
	assert(!stLocale.empty() && "Empty locale");

	m_stLocale = stLocale;
	if (m_connected && mysql_set_character_set(&m_hDB, m_stLocale.c_str()))
	{
		sys_err("Failed to set locale %s with errno %u %s", m_stLocale.c_str(), mysql_errno(&m_hDB), mysql_error(&m_hDB));
	}
}

SQLMsg* CAsyncSQL::DirectQuery(const char* c_pszQuery)
{
	assert(!m_asyncThread.joinable() && "DirectQuery not allowed");
	assert(m_connected && "We were never connected");

//	sys_log(0, "Query: '%s'", c_pszQuery);

	auto p = new SQLMsg;
	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;

	if (mysql_real_query(&m_hDB, p->stQuery.c_str(), static_cast<uint32_t>(p->stQuery.length())))
	{
		sys_err("DirectQuery failed with error: %u(%s) query: '%s", mysql_errno(&m_hDB), mysql_error(&m_hDB), p->stQuery.c_str());
		p->uiSQLErrno = mysql_errno(&m_hDB);
	}

	p->Store();
	return p;
}

void CAsyncSQL::AsyncQuery(const char* c_pszQuery)
{
	auto p = new SQLMsg;
	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;

	PushQuery(p);
}

void CAsyncSQL::ReturnQuery(const char* c_pszQuery, void* pvUserData)
{
	auto p = new SQLMsg;
	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;
	p->bReturn = true;
	p->pvUserData = pvUserData;

	PushQuery(p);
}

void CAsyncSQL::PushResult(SQLMsg* p)
{
	std::lock_guard<std::mutex> lock(m_resultMutex);
	m_queue_result.push(p);
}

bool CAsyncSQL::PopResult(SQLMsg** pp)
{
	std::lock_guard<std::mutex> lock(m_resultMutex);

	if (m_queue_result.empty())
		return false;

	*pp = m_queue_result.front();
	m_queue_result.pop();
	return true;
}

void CAsyncSQL::PushQuery(SQLMsg* p)
{
	{
		std::lock_guard<std::mutex> lock(m_queryMutex);
		m_queue_query.push(p);
	}

	m_sem.Release();
}

std::queue<SQLMsg*> CAsyncSQL::PopPendingQueries()
{
	std::lock_guard<std::mutex> lock(m_queryMutex);

	std::queue<SQLMsg*> queue;
	swap(queue, m_queue_query);

	return queue;
}

uint32_t CAsyncSQL::CountQuery() const
{
	return static_cast<uint32_t>(m_queue_query.size());
}

uint32_t CAsyncSQL::CountResult() const
{
	return static_cast<uint32_t>(m_queue_result.size());
}

void CAsyncSQL::ChildLoop()
{
	while (!m_bEnd) 
	{
		m_sem.Wait();

		auto queue = PopPendingQueries();
		while (!queue.empty()) 
		{
			auto p = queue.front();

			const auto startTime = get_dword_time();
			if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length())) 
			{
				p->uiSQLErrno = mysql_errno(&m_hDB);

				const bool retry = ShouldRetry(p->uiSQLErrno);
				sys_err("mysql_query(%s) failed with %s (retry %d)", p->stQuery.c_str(), mysql_error(&m_hDB), retry);

				if (retry)
					continue;
			}

			// 0.5? ?? ???? ??? ???
			const auto runTime = get_dword_time() - startTime;
			if (runTime > 5000)
				sys_log(1, "Query %s took %d ms", p->stQuery.c_str(), runTime);

			queue.pop();

			p->Store();

			if (p->bReturn)
				PushResult(p);
			else
				delete p;

			++m_iQueryFinished;
		}
	}

	auto queue = PopPendingQueries();
	while (!queue.empty()) 
	{
		auto p = queue.front();
		if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length())) 
		{
			p->uiSQLErrno = mysql_errno(&m_hDB);

			const bool retry = ShouldRetry(p->uiSQLErrno);
			sys_err("mysql_query(%s) failed with %s (retry %d)", p->stQuery.c_str(), mysql_error(&m_hDB), retry);

			if (retry)
				continue;
		}

		sys_log(0, "QUERY_FLUSH: %s", p->stQuery.c_str());

		queue.pop();

		p->Store();

		if (p->bReturn)
			PushResult(p);
		else
			delete p;

		++m_iQueryFinished;
	}
}

int32_t CAsyncSQL::CountQueryFinished() const
{
	return m_iQueryFinished;
}

void CAsyncSQL::ResetQueryFinished()
{
	m_iQueryFinished = 0;
}

MYSQL * CAsyncSQL::GetSQLHandle()
{
	return &m_hDB;
}

size_t CAsyncSQL::EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize)
{
	if (0 == srcSize)
	{
		memset(dst, 0, dstSize);
		return 0;
	}

	if (0 == dstSize)
		return 0;

	if (dstSize < srcSize * 2 + 1)
	{
		// \0? ????? ?? ???? 256 ???? ???? ??? ??
		char tmp[256];
		size_t tmpLen = sizeof(tmp) > srcSize ? srcSize : sizeof(tmp); // ? ?? ?? ??
		strlcpy(tmp, src, tmpLen);

		sys_err(
			"Buffer overflow (dstSize {0} srcSize {1} src{2}: {3})",
			dstSize, srcSize,
			tmpLen != srcSize ? " (trimmed to 255 characters)" : "",
			tmp);

		dst[0] = '\0';
		return 0;
	}

	return mysql_real_escape_string(&m_hDB, dst, src, srcSize);
}

