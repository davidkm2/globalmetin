#pragma once

class CNetBase
{
    public:
	CNetBase();
	virtual ~CNetBase();

    protected:
	static LPFDWATCH	m_fdWatcher;
};

class CNetPoller : public CNetBase, public singleton<CNetPoller>
{
    public:
	CNetPoller();
	virtual ~CNetPoller();

	bool	Create();
	void	Destroy();
};

