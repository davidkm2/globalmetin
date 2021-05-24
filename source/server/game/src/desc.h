#ifndef __INC_METIN_II_GAME_DESC_H__
#define __INC_METIN_II_GAME_DESC_H__

#include "constants.h"
#include "input.h"

#define MAX_ALLOW_USER                  4096
//#define MAX_INPUT_LEN			2048
#define MAX_INPUT_LEN			65536

#define HANDSHAKE_RETRY_LIMIT		32

class CInputProcessor;

enum EDescType
{
	DESC_TYPE_ACCEPTOR,
	DESC_TYPE_CONNECTOR
};

class CLoginKey
{
	public:
		CLoginKey(DWORD dwKey, LPDESC pkDesc) : m_dwKey(dwKey), m_pkDesc(pkDesc)
		{
			m_dwExpireTime = 0;
		}

		void Expire()
		{
			m_dwExpireTime = get_dword_time();
			m_pkDesc = NULL;
		}

		operator DWORD() const
		{
			return m_dwKey;
		}

		DWORD   m_dwKey;
		DWORD   m_dwExpireTime;
		LPDESC  m_pkDesc;
};

class DESC
{
	public:
		EVENTINFO(desc_event_info)
		{
			LPDESC desc;

			desc_event_info()
			: desc(0)
			{
			}
		};

	public:
		DESC();
		virtual ~DESC();

		virtual BYTE		GetType() { return DESC_TYPE_ACCEPTOR; }
		virtual void		Destroy();
		virtual void		SetPhase(int _phase);

		void			FlushOutput();

		bool			Setup(LPFDWATCH _fdw, socket_t _fd, const struct sockaddr_in & c_rSockAddr, DWORD _handle, DWORD _handshake);

		socket_t		GetSocket() const	{ return m_sock; }
		const char *	GetHostName()		{ return m_stHost.c_str(); }
		WORD			GetPort()	{ return m_wPort; }

		void			SetP2P(const char * h, WORD w, BYTE b) { m_stP2PHost = h; m_wP2PPort = w; m_bP2PChannel = b; }
		const char *	GetP2PHost()		{ return m_stP2PHost.c_str();	}
		WORD			GetP2PPort() const		{ return m_wP2PPort; }
		BYTE			GetP2PChannel() const	{ return m_bP2PChannel;	}

		void			BufferedPacket(const void * c_pvData, int iSize);
		void			Packet(const void * c_pvData, int iSize);
		void			LargePacket(const void * c_pvData, int iSize);

		int			ProcessInput();		// returns -1 if error
		int			ProcessOutput();	// returns -1 if error

		CInputProcessor	*	GetInputProcessor()	{ return m_pInputProcessor; }

		DWORD			GetHandle() const	{ return m_dwHandle; }
		LPBUFFER		GetOutputBuffer()	{ return m_lpOutputBuffer; }

		void			BindAccountTable(TAccountTable * pTable);
		TAccountTable &		GetAccountTable()	{ return m_accountTable; }

		void			BindCharacter(LPCHARACTER ch);
		LPCHARACTER		GetCharacter()		{ return m_lpCharacter; }

		bool			IsPhase(int phase) const	{ return m_iPhase == phase ? true : false; }

		const struct sockaddr_in & GetAddr()		{ return m_SockAddr;	}

		void			Log(const char * format, ...);

		// �ڵ彦��ũ (�ð� ����ȭ)
		void			StartHandshake(DWORD _dw);
		void			SendHandshake(DWORD dwCurTime, long lNewDelta);
		bool			HandshakeProcess(DWORD dwTime, long lDelta, bool bInfiniteRetry=false);
		bool			IsHandshaking();

		DWORD			GetHandshake() const	{ return m_dwHandshake; }
		DWORD			GetClientTime();

		void			SetSecurityKey(const DWORD * c_pdwKey);
		const uint32_t*	GetEncryptionKey() const { return &m_adwEncryptionKey[0]; }
		const uint32_t*	GetDecryptionKey() const { return &m_adwDecryptionKey[0]; }

		// ����
		BYTE			GetEmpire();

		// for p2p
		void			SetRelay(const char * c_pszName);
		bool			DelayedDisconnect(int iSec);
		void			DisconnectOfSameLogin();

		void			SetAdminMode();
		bool			IsAdminMode();		// Handshake ���� ���� ����� �����ֳ�?

		void			SetPong(bool b);
		bool			IsPong();

		void			SendLoginSuccessPacket();
		//void			SendServerStatePacket(int nIndex);

		void			SetLoginKey(DWORD dwKey);
		void			SetLoginKey(CLoginKey * pkKey);
		DWORD			GetLoginKey();

		void			SetClientVersion(const char * c_pszTimestamp) { m_stClientVersion = c_pszTimestamp; }
		const char *		GetClientVersion() { return m_stClientVersion.c_str(); }

		bool			isChannelStatusRequested() const { return m_bChannelStatusRequested; }
		void			SetChannelStatusRequested(bool bChannelStatusRequested) { m_bChannelStatusRequested = bChannelStatusRequested; }

	protected:
		void			Initialize();

	protected:
		CInputProcessor *	m_pInputProcessor;
		CInputClose		m_inputClose;
		CInputHandshake	m_inputHandshake;
		CInputLogin		m_inputLogin;
		CInputMain		m_inputMain;
		CInputAuth		m_inputAuth;

		LPFDWATCH		m_lpFdw;
		socket_t		m_sock;
		int				m_iPhase;
		DWORD			m_dwHandle;

		std::string		m_stHost;
		WORD			m_wPort;
		time_t			m_LastTryToConnectTime;

		LPBUFFER		m_lpInputBuffer;
		int				m_iMinInputBufferLen;

		DWORD			m_dwHandshake;
		DWORD			m_dwHandshakeSentTime;
		int				m_iHandshakeRetry;
		DWORD			m_dwClientTime;
		bool			m_bHandshaking;

		LPBUFFER		m_lpBufferedOutputBuffer;
		LPBUFFER		m_lpOutputBuffer;

		LPEVENT			m_pkPingEvent;
		LPCHARACTER		m_lpCharacter;
		TAccountTable		m_accountTable;

		struct sockaddr_in	m_SockAddr;

		FILE *			m_pLogFile;
		std::string		m_stRelayName;

		std::string		m_stP2PHost;
		WORD			m_wP2PPort;
		BYTE			m_bP2PChannel;

		bool			m_bAdminMode; // Handshake ���� ���� ����� �����ֳ�?
		bool			m_bPong;

		CLoginKey *		m_pkLoginKey;
		DWORD			m_dwLoginKey;

		BYTE                    m_bCRCMagicCubeIdx;

		std::string		m_stClientVersion;

		std::string		m_Login;

		bool			m_bDestroyed;
		bool			m_bChannelStatusRequested;

		bool			m_bEncrypted;
		uint32_t			m_adwDecryptionKey[4];
		uint32_t			m_adwEncryptionKey[4];

	public:
		LPEVENT			m_pkDisconnectEvent;

	public:
		void SetLogin( const std::string & login ) { m_Login = login; }
		void SetLogin( const char * login ) { m_Login = login; }
		const std::string& GetLogin() { return m_Login; }
};

#endif
