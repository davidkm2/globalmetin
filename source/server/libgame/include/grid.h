#ifndef __INC_GRID_H__
#define __INC_GRID_H__

class CGrid
{
    public:
	CGrid(int32_t w, int32_t h);
	CGrid(CGrid * pkGrid, int32_t w, int32_t h);
	~CGrid();

	void		UpdateSize(int w, int h);
	void		Clear();
	int32_t		FindBlank(int32_t w, int32_t h);
	bool		IsEmpty(int32_t iPos, int32_t w, int32_t h);
	bool		Put(int32_t iPos, int32_t w, int32_t h);
	void		Get(int32_t iPos, int32_t w, int32_t h);
	void		Print();
	uint32_t	GetSize();

	int GetPosColumn(int iPos);
	int GetPosRow(int iPos);

    protected:
	int32_t	m_iWidth;
	int32_t	m_iHeight;

	char *	m_pGrid;
};

#endif
