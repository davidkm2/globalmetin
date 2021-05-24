#pragma once

class CPythonMessenger : public CSingleton<CPythonMessenger>
{
	public:
		typedef std::set<std::string> TFriendNameMap;
#ifdef ENABLE_MESSENGER_BLOCK
		typedef std::set<std::string> TBlockNameMap;
#endif
		typedef std::map<std::string, BYTE> TGuildMemberStateMap;

		enum EMessengerGroupIndex
		{
			MESSENGER_GRUOP_INDEX_FRIEND,
			MESSENGER_GRUOP_INDEX_GUILD,
#ifdef ENABLE_MESSENGER_BLOCK
			MESSENGER_GROUP_INDEX_BLOCK,
#endif
		};

	public:
		CPythonMessenger();
		virtual ~CPythonMessenger();

		void Destroy();
		void RemoveFriend(const char * c_szKey);
		void OnFriendLogin(const char * c_szKey);
		void OnFriendLogout(const char * c_szKey);
		BOOL IsFriendByKey(const char * c_szKey);
		BOOL IsFriendByName(const char * c_szName);

#ifdef ENABLE_MESSENGER_BLOCK
		void RemoveBlock(const char * c_szKey);
		void OnBlockLogin(const char * c_szKey);
		void OnBlockLogout(const char * c_szKey);
		BOOL IsBlockByKey(const char * c_szKey);
		BOOL IsBlockFriendByName(const char * c_szName);
#endif
		void AppendGuildMember(const char * c_szName);
		void RemoveGuildMember(const char * c_szName);
		void RemoveAllGuildMember();
		void LoginGuildMember(const char * c_szName);
		void LogoutGuildMember(const char * c_szName);
		void RefreshGuildMember();

		void SetMessengerHandler(PyObject* poHandler);

#ifdef ENABLE_MAILBOX
		TFriendNameMap GetFriendMap() { return m_FriendNameMap; };
#endif

	protected:
		TFriendNameMap m_FriendNameMap;
		TGuildMemberStateMap m_GuildMemberStateMap;
#ifdef ENABLE_MESSENGER_BLOCK
		TBlockNameMap m_BlockNameMap;
#endif

	private:
		PyObject * m_poMessengerHandler;
};

