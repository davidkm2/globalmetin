#ifndef	__HEADER_MOUNT_SYSTEM__
#define	__HEADER_MOUNT_SYSTEM__
#include "stdafx.h"

class CHARACTER;

class CMountActor
{
	protected:
		friend class CMountSystem;
		CMountActor(LPCHARACTER owner, DWORD vnum);
		
		virtual ~CMountActor();
		virtual bool	Update(DWORD deltaTime);
		
	protected:
		virtual bool	_UpdateFollowAI();
		virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);
		
	private:
		bool Follow(float fMinDistance = 50.f);
		
	public:
		LPCHARACTER		GetCharacter()	const					{ return m_pkCharacter; }
		LPCHARACTER		GetOwner()	const						{ return m_pkOwner; }
		DWORD			GetVnum() const							{ return m_dwVnum; }
		DWORD			GetVID() const							{ return m_dwVID; }
		
		void			SetName(const char* MountName);
		DWORD			Mount();
		DWORD			Unmount();

		void			Summon(const char* MounName, DWORD mobVnum);
		void			Unsummon();

		bool			IsSummoned() const { return 0 != m_pkCharacter; }
		bool			IsMounting() const { return m_dwMounted; }
	
		void			UpdateTime();

	private:
		DWORD			m_dwVnum;
		DWORD			m_dwVID;
		DWORD			m_dwLastActionTime;
		DWORD			m_dwUpdatePeriod;
		DWORD			m_dwLastUpdateTime;
		short			m_OMoveSpeed;
		bool			m_dwMounted;
		std::string		m_Name;

		LPCHARACTER		m_pkCharacter;
		LPCHARACTER		m_pkOwner;
};

class CMountSystem
{
	public:
		typedef	std::map<DWORD,	CMountActor*> TMountActorMap;
		
	public:
		CMountSystem(LPCHARACTER owner);
		virtual ~CMountSystem();

		CMountActor*	GetByVID(DWORD vid) const;
		CMountActor*	GetByVnum(DWORD vnum) const;

		bool		Update(DWORD deltaTime);
		void		Destroy();

		size_t		CountSummoned() const;

	public:
		void		SetUpdatePeriod(DWORD ms);

		CMountActor*	Summon(DWORD mobVnum, const char* MountName);
		CMountActor*	Mount(DWORD mobVnum);
		CMountActor*	Unmount(DWORD mobVnum);
		
		void		Unsummon(DWORD mobVnum, bool bDeleteFromList = false);
		void		Unsummon(CMountActor* MountActor, bool bDeleteFromList = false);

		CMountActor* 	Mount(DWORD mobVnum, const char* MountName);
		
		void		DeleteMount(DWORD mobVnum);
		void		DeleteMount(CMountActor* MountActor);
		bool		IsActiveMount();
		bool		IsMounting(DWORD mobVnum);
		void		UpdateTime();
	
	private:
		TMountActorMap	m_MountActorMap;
		LPCHARACTER		m_pkOwner;	
		
		DWORD			m_dwUpdatePeriod;
		DWORD			m_dwLastUpdateTime;
		LPEVENT			m_pkMountSystemUpdateEvent;
};

#endif

