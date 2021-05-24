#ifndef __INC_ASYNCSQL_H__
#define __INC_ASYNCSQL_H__

#include "../../libthecore/include/stdafx.h"
#include "../../libthecore/include/log.h"

#include <string>
#include <queue>
#include <vector>
#include <map>
#include <mysql.h>
#include <errmsg.h>
#include <mysqld_error.h>

#include <thread>
#include <mutex>

#include "Semaphore.h"

#define ASQL_QUERY_MAX_LEN 8192

//Safe strlcpy method for mysql returning a nullptr.
inline void m_strlcpy(char* dst, char* src, size_t size)
{
	if (src == nullptr) { //if we got a nullptr set to \0 termination char
		strlcpy(dst, "\0", 1);
		return;
	}


	strlcpy(dst, src, size);
}

struct SQLResult
{
	SQLResult();
	~SQLResult();

	MYSQL_RES* pSQLResult;
	uint32_t uiNumRows;
	uint32_t uiAffectedRows;
	uint32_t uiInsertID;
};

struct SQLMsg
{
	SQLMsg();
	~SQLMsg();

	void Store();

	SQLResult* Get();
	bool Next();

	MYSQL* m_pkSQL;
	int32_t iID;
	std::string stQuery;

	std::vector<SQLResult*> vec_pkResult; // result ??
	uint32_t uiResultPos; // ?? result ??

	void* pvUserData;
	bool bReturn;

	uint32_t uiSQLErrno;
};

class CAsyncSQL
{
public:
	CAsyncSQL();
	~CAsyncSQL();

	void Quit();

	bool Setup(const char* c_pszHost, const char* c_pszUser,
		const char* c_pszPassword,
		const char* c_pszDB, const char* c_pszLocale,
		bool bNoThread = false, int32_t iPort = 0);

	bool Setup(CAsyncSQL* sql, bool bNoThread = false);

	void SetLocale(const std::string& stLocale);

	bool Connect();

	void AsyncQuery(const char* c_pszQuery);
	void ReturnQuery(const char* c_pszQuery, void* pvUserData);
	SQLMsg* DirectQuery(const char* c_pszQuery);

	uint32_t CountQuery() const;
	uint32_t CountResult() const;

	void PushResult(SQLMsg* p);
	bool PopResult(SQLMsg** pp);

	void ChildLoop();

	MYSQL* GetSQLHandle();

	int32_t CountQueryFinished() const;
	void ResetQueryFinished();

	size_t EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize);

protected:
	void PushQuery(SQLMsg* p);

	std::queue<SQLMsg*> PopPendingQueries();

	MYSQL m_hDB;

	std::string m_stHost;
	std::string m_stUser;
	std::string m_stPassword;
	std::string m_stDB;
	std::string m_stLocale;
	bool m_connected;

	int32_t	m_iMsgCount;
	int32_t m_iPort;

	std::queue<SQLMsg*> m_queue_query;
	std::queue<SQLMsg*> m_queue_result;

	volatile bool m_bEnd;

	std::thread m_asyncThread;
	std::mutex m_queryMutex;
	std::mutex m_resultMutex;

	CSemaphore m_sem;

	int32_t	m_iQueryFinished;
};

typedef CAsyncSQL CAsyncSQL2;
#endif
