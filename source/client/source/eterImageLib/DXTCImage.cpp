#include "StdAfx.h"
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>

#include "../EterBase/MappedFile.h"
#include "../EterBase/Debug.h"

#include "DXTCImage.h"

struct DXTColBlock
{
	WORD col0;
	WORD col1;
	BYTE row[4];
};

struct DXTAlphaBlockExplicit
{
	WORD row[4];
};

struct DXTAlphaBlock3BitLinear
{
	BYTE alpha0;
	BYTE alpha1;

	BYTE stuff[6];
};
struct Color8888
{
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE a;
};

struct Color565
{
	unsigned nBlue  : 5;
	unsigned nGreen : 6;
	unsigned nRed	: 5;
};
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
				((DWORD)(BYTE) (ch0       ) | ((DWORD)(BYTE) (ch1) <<  8) | \
				((DWORD)(BYTE) (ch2) << 16) | ((DWORD)(BYTE) (ch3) << 24))
#endif 

CDXTCImage::CDXTCImage()
{
	Initialize();
}

CDXTCImage::~CDXTCImage()
{
}

void CDXTCImage::Initialize()
{
	m_nWidth = 0;
	m_nHeight = 0;

	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_pbCompBufferByLevels[i] = NULL;
}

void CDXTCImage::Clear()
{
	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_bCompVector[i].clear();

	Initialize();
}

bool CDXTCImage::LoadFromFile(const char * filename)
{
	char * exts[] = { ".DDS" };
	int next = 1;

	static char fileupper[MAX_PATH+1];

	strncpy(fileupper, filename, MAX_PATH);
	_strupr(fileupper);

	int i;
	bool knownformat = false;

	for (i = 0; i < next; ++i)
	{
		char * found = strstr(fileupper, exts[0]);

		if (found != NULL)
		{
			knownformat = true;
			break;
		}
	}

	if (knownformat == false)
	{
		Tracef("Unknown file format encountered! [%s]\n", filename);
		return(false);
	}

	CMappedFile mappedFile;
	LPCVOID pvMap;

	if (!mappedFile.Create(filename, &pvMap, 0, 0))
	{
		Tracef("Can't open file for reading! [%s]\n", filename);
		return false;
	}

	return LoadFromMemory((const BYTE*) pvMap);
}

bool CDXTCImage::LoadHeaderFromMemory(const BYTE * c_pbMap)
{
	DWORD dwMagic;
	dwMagic = *(DWORD *) c_pbMap;
	c_pbMap += sizeof(DWORD);
	DDSURFACEDESC2 ddsd; 
	memcpy(&ddsd, c_pbMap, sizeof(DDSURFACEDESC2));
	c_pbMap += sizeof(DDSURFACEDESC2);
	m_bMipTexture = (ddsd.dwMipMapCount > 0) ? TRUE : FALSE;
	m_xddPixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
	m_xddPixelFormat.dwFourCC = ddsd.ddpfPixelFormat.dwFourCC;
	m_xddPixelFormat.dwSize = ddsd.ddpfPixelFormat.dwSize;
	m_xddPixelFormat.dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_xddPixelFormat.dwRGBAlphaBitMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	m_xddPixelFormat.dwRBitMask = ddsd.ddpfPixelFormat.dwRBitMask;
	m_xddPixelFormat.dwGBitMask = ddsd.ddpfPixelFormat.dwGBitMask;
	m_xddPixelFormat.dwBBitMask = ddsd.ddpfPixelFormat.dwBBitMask;

	DecodePixelFormat(m_strFormat, &m_xddPixelFormat);

	if (m_CompFormat != PF_DXT1 &&
		m_CompFormat != PF_DXT3 &&
		m_CompFormat != PF_DXT5)
	{
		return false;
	}

	if (ddsd.dwMipMapCount > MAX_MIPLEVELS)
		ddsd.dwMipMapCount = MAX_MIPLEVELS;

	m_nWidth		= ddsd.dwWidth;
	m_nHeight		= ddsd.dwHeight;

	m_dwMipMapCount = std::max<DWORD>(1, ddsd.dwMipMapCount);
	m_dwFlags		= ddsd.dwFlags;

	if (ddsd.dwFlags & DDSD_PITCH)
	{
		m_lPitch = ddsd.lPitch;
		m_pbCompBufferByLevels[0] = c_pbMap;
	}
	else
	{
		m_lPitch = ddsd.dwLinearSize;

		if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (DWORD dwLinearSize = ddsd.dwLinearSize, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_pbCompBufferByLevels[i] = c_pbMap;
				c_pbMap += dwLinearSize;
			}
		}
		else
		{
			m_pbCompBufferByLevels[0] = c_pbMap;
		}
	}

	return true;
}
bool CDXTCImage::LoadFromMemory(const BYTE * c_pbMap)
{
	if (!LoadHeaderFromMemory(c_pbMap))
		return false;

	if (m_dwFlags & DDSD_PITCH)
	{
		DWORD dwBytesPerRow = m_nWidth * m_xddPixelFormat.dwRGBBitCount / 8;

		m_nCompSize = m_lPitch * m_nHeight;
		m_nCompLineSz = dwBytesPerRow;

		m_bCompVector[0].resize(m_nCompSize);
		BYTE * pDest = &m_bCompVector[0][0];

		c_pbMap = m_pbCompBufferByLevels[0];

		for (int yp = 0; yp < m_nHeight; ++yp)
		{
			memcpy(pDest, c_pbMap, dwBytesPerRow);
			pDest += m_lPitch;
			c_pbMap += m_lPitch;
		}
	}
	else
	{
		if (m_dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (DWORD dwLinearSize = m_lPitch, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_bCompVector[i].resize(dwLinearSize);
				Copy(i, &m_bCompVector[i][0], dwLinearSize);
			}
		}
		else
		{
			m_bCompVector[0].resize(m_lPitch);
			Copy(0, &m_bCompVector[0][0], m_lPitch);
		}
	}
	return true;
}

bool CDXTCImage::Copy(int miplevel, BYTE * pbDest, long lDestPitch)
{
	if (!(m_dwFlags & DDSD_MIPMAPCOUNT))
		if (miplevel)
			return false;

	memcpy(pbDest, m_pbCompBufferByLevels[miplevel], m_lPitch >> (miplevel * 2));
	pbDest += lDestPitch;
	return true;
}

void CDXTCImage::Unextract(BYTE * pbDest, int /*iWidth*/, int /*iHeight*/, int iPitch)
{
	if (!m_pbCompBufferByLevels[0])
		return;

	DXTColBlock * pBlock;
	BYTE * pPos = (BYTE *) &m_pbCompBufferByLevels[0][0];
	int xblocks = m_nWidth / 4;
	int yblocks = (m_nHeight / 4) * ((iPitch / m_nWidth) / 2);

	for (int y = 0; y < yblocks; ++y)
	{
		pBlock = (DXTColBlock*) (pPos + y * xblocks * 8);

		memcpy(pbDest, pBlock, xblocks * 8);
		pbDest += xblocks * 8;
	}
}

void CDXTCImage::Decompress(int miplevel, DWORD * pdwDest)
{
	switch (m_CompFormat)
	{
		case PF_DXT1:
			DecompressDXT1(miplevel, pdwDest);
			break;

		case PF_DXT3:
			DecompressDXT3(miplevel, pdwDest);
			break;

		case PF_DXT5:
			DecompressDXT5(miplevel, pdwDest);
			break;

		case PF_ARGB:
			DecompressARGB(miplevel, pdwDest);
			break;

		case PF_UNKNOWN:
			break;
	}
}

inline void GetColorBlockColors(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1,
								Color8888 * col_2, Color8888 * col_3,
								WORD & wrd)
{
	Color565 * pCol;

	pCol = (Color565*) & (pBlock->col0);

	col_0->a = 0xff;
	col_0->r = pCol->nRed;
	col_0->r <<= 3;
	col_0->g = pCol->nGreen;
	col_0->g <<= 2;
	col_0->b = pCol->nBlue;
	col_0->b <<= 3;

	pCol = (Color565*) & (pBlock->col1);
	col_1->a = 0xff;
	col_1->r = pCol->nRed;
	col_1->r <<= 3;
	col_1->g = pCol->nGreen;
	col_1->g <<= 2;
	col_1->b = pCol->nBlue;
	col_1->b <<= 3;

	if (pBlock->col0 > pBlock->col1)
	{
		wrd = (WORD) (((WORD) col_0->r * 2 + (WORD) col_1->r) / 3);
		col_2->r = (BYTE)wrd;

		wrd = (WORD) (((WORD) col_0->g * 2 + (WORD) col_1->g) / 3);
		col_2->g = (BYTE)wrd;

		wrd = (WORD) (((WORD) col_0->b * 2 + (WORD) col_1->b) / 3);
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		wrd = (WORD) (((WORD) col_0->r + (WORD) col_1->r * 2) / 3);
		col_3->r = (BYTE)wrd;

		wrd = (WORD) (((WORD) col_0->g + (WORD) col_1->g * 2) / 3);
		col_3->g = (BYTE)wrd;

		wrd = (WORD) (((WORD) col_0->b + (WORD) col_1->b * 2) / 3);
		col_3->b = (BYTE)wrd;
		col_3->a = 0xff;
	}
	else
	{
		wrd = (WORD) (((WORD) col_0->r + (WORD) col_1->r) / 2);
		col_2->r = (BYTE)wrd;
		wrd = (WORD) (((WORD) col_0->g + (WORD) col_1->g) / 2);
		col_2->g = (BYTE)wrd;
		wrd = (WORD) (((WORD) col_0->b + (WORD) col_1->b) / 2);
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		col_3->r = 0x00;
		col_3->g = 0x00;
		col_3->b = 0x00;
		col_3->a = 0x00;
	}
} 
inline void DecodeColorBlock(DWORD * pImPos,
							 DXTColBlock * pColorBlock,
							 int width,
							 DWORD * col_0,
							 DWORD * col_1,
							 DWORD * col_2,
							 DWORD * col_3)
{

	DWORD bits;
	int y, n;
	const DWORD masks[] = { 3, 12, 3 << 4, 3 << 6 };
	const int   shift[] = { 0, 2, 4, 6 };
	for (y = 0; y < 4; ++y, pImPos += width - 4) 
	{
		for (n = 0; n < 4; ++n)
		{
			bits = pColorBlock->row[y] & masks[n];
			bits >>= shift[n];

			switch (bits)
			{
				case 0:
					*pImPos = *col_0;
					pImPos++; 
					break;

				case 1:
					*pImPos = *col_1;
					pImPos++;
					break;

				case 2:
					*pImPos = *col_2;
					pImPos++;
					break;

				case 3:
					*pImPos = *col_3;
					pImPos++;
					break;

				default:
					Tracef("Your logic is jacked! bits == 0x%x\n", bits);
					pImPos++;
					break;
			}
		}
	}
}

inline void DecodeAlphaExplicit(DWORD * pImPos, DXTAlphaBlockExplicit * pAlphaBlock,
								int width, DWORD alphazero)
{
	int row, pix;

	WORD wrd;

	Color8888 col;
	col.r = col.g = col.b = 0;
	for (row = 0; row < 4; row++, pImPos += width - 4)
	{

		wrd = pAlphaBlock->row[row];
		for (pix = 0; pix < 4; ++pix)
		{

			*pImPos &= alphazero;

			col.a = (BYTE) (wrd & 0x000f);
			col.a = (BYTE) (col.a | (col.a << 4));
			*pImPos |= *((DWORD*)&col);

			wrd >>= 4; 

			pImPos++; 
		}
	}
}

static BYTE			gBits[4][4];
static WORD			gAlphas[8];
static Color8888	gACol[4][4];

inline void DecodeAlpha3BitLinear(DWORD * pImPos, DXTAlphaBlock3BitLinear * pAlphaBlock,
								  int width, DWORD alphazero)
{
	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;
	if (gAlphas[0] > gAlphas[1])
	{
		gAlphas[2] = (WORD) ((6 * gAlphas[0] +     gAlphas[1]) / 7);
		gAlphas[3] = (WORD) ((5 * gAlphas[0] + 2 * gAlphas[1]) / 7);
		gAlphas[4] = (WORD) ((4 * gAlphas[0] + 3 * gAlphas[1]) / 7);
		gAlphas[5] = (WORD) ((3 * gAlphas[0] + 4 * gAlphas[1]) / 7);
		gAlphas[6] = (WORD) ((2 * gAlphas[0] + 5 * gAlphas[1]) / 7);
		gAlphas[7] = (WORD) ((    gAlphas[0] + 6 * gAlphas[1]) / 7);
	}
	else
	{
		gAlphas[2] = (WORD) ((4 * gAlphas[0] +     gAlphas[1]) / 5);
		gAlphas[3] = (WORD) ((3 * gAlphas[0] + 2 * gAlphas[1]) / 5);
		gAlphas[4] = (WORD) ((2 * gAlphas[0] + 3 * gAlphas[1]) / 5);
		gAlphas[5] = (WORD) ((   gAlphas[0] + 4 * gAlphas[1]) / 5);
		gAlphas[6] = 0;
		gAlphas[7] = 255;
	}
	const DWORD mask = 0x00000007;
	DWORD bits = *((DWORD*) & (pAlphaBlock->stuff[0]));

	gBits[0][0] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[0][1] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[0][2] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[0][3] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[1][0] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[1][1] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[1][2] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[1][3] = (BYTE) (bits & mask);
	bits = *((DWORD*) & (pAlphaBlock->stuff[3]));

	gBits[2][0] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[2][1] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[2][2] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[2][3] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[3][0] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[3][1] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[3][2] = (BYTE) (bits & mask);
	bits >>= 3;
	gBits[3][3] = (BYTE) (bits & mask);
	int row, pix;

	for (row = 0; row < 4; ++row)
	{
		for (pix = 0; pix < 4; ++pix)
		{
			gACol[row][pix].a = (BYTE) gAlphas[gBits[row][pix]];

			assert(gACol[row][pix].r == 0);
			assert(gACol[row][pix].g == 0);
			assert(gACol[row][pix].b == 0);
		}
	}
	for (row = 0; row < 4; ++row, pImPos += width - 4)
	{

		for (pix = 0; pix < 4; ++pix)
		{

			*pImPos &= alphazero;
			*pImPos |= *((DWORD*) &(gACol[row][pix]));
			pImPos++;
		}
	}
}

void CDXTCImage::DecompressDXT1(int miplevel, DWORD * pdwDest)
{
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
	{
		Tracef("****** warning width not div by 4!  %d\n", ddsd.dwWidth);
	}

	if ((ddsd.dwHeight % 4) != 0)
	{
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);
	}

	Tracef("end check\n");
#endif
	UINT nWidth = m_nWidth >> miplevel;
	UINT nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int		x, y;
	DWORD * pBase	= (DWORD *) pdwDest;
	WORD  * pPos	= (WORD *) &m_bCompVector[miplevel][0];;
	DWORD * pImPos;

	DXTColBlock	* pBlock;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	for (y = 0; y < yblocks; ++y)
	{

		pBlock = (DXTColBlock *) ((DWORD) pPos + y * xblocks * 8);

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{

			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			pImPos = (DWORD *) ((DWORD) pBase + x*16 + (y*4) * nWidth * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, (DWORD *)&col_0, (DWORD *)&col_1, (DWORD *)&col_2, (DWORD *)&col_3);
		}
	}
}

void CDXTCImage::DecompressDXT3(int miplevel, DWORD* pdwDest)
{
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
	{
		Tracef("****** warning width not div by 4! %d\n", ddsd.dwWidth);
	}

	if ((ddsd.dwHeight % 4) != 0)
	{
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);
	}

	Tracef("end check\n");
#endif
	UINT nWidth = m_nWidth >> miplevel;
	UINT nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int		x, y;
	DWORD * pBase	= (DWORD *) pdwDest;
	WORD  * pPos	= (WORD *) &m_bCompVector[miplevel][0]; 
	DWORD * pImPos;

	DXTColBlock	* pBlock;
	DXTAlphaBlockExplicit * pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;

	DWORD alphazero = *((DWORD *) &col_0);

	for (y = 0; y < yblocks; ++y)
	{
		pBlock = (DXTColBlock *) ((DWORD) (pPos + y * xblocks * 16));

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{
			pAlphaBlock = (DXTAlphaBlockExplicit *) pBlock;
			pBlock++;
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
			pImPos = (DWORD *) ((DWORD) (pBase + x * 16 + (y * 4) * nWidth * 4));

			DecodeColorBlock(pImPos,
							 pBlock,
							 nWidth,
							 (DWORD *) &col_0, (DWORD *) &col_1, (DWORD *) &col_2, (DWORD *) &col_3);
			DecodeAlphaExplicit(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
}

void CDXTCImage::DecompressDXT5(int level, DWORD * pdwDest)
{
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
	{
		Tracef("****** warning width not div by 4! %d\n", ddsd.dwWidth);
	}

	if ((ddsd.dwHeight % 4) != 0)
	{
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);
	}

	Tracef("end check\n");
#endif
	UINT nWidth = m_nWidth >> level;
	UINT nHeight = m_nHeight >> level;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	int x, y;

	DWORD * pBase = (DWORD *) pdwDest;
	WORD  * pPos = pPos = (WORD *) &m_bCompVector[level][0]; 
	DWORD * pImPos;

	DXTColBlock	* pBlock;
	DXTAlphaBlock3BitLinear * pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	DWORD alphazero = *((DWORD *) &col_0);
	for (y = 0; y < yblocks; ++y)
	{
		pBlock = (DXTColBlock*) ((DWORD) (pPos + y * xblocks * 16));

		for (x = 0; x < xblocks; ++x, ++pBlock)
		{
			pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;
			pBlock++;
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
			pImPos = (DWORD *) ((DWORD) (pBase + x * 16 + (y * 4) * nWidth * 4));
			DecodeColorBlock(pImPos, pBlock, nWidth, (DWORD *)&col_0, (DWORD *)&col_1, (DWORD *)&col_2, (DWORD *)&col_3);
			DecodeAlpha3BitLinear(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
}

void CDXTCImage::DecompressARGB(int level, DWORD * pdwDest)
{
	UINT lPitch = m_lPitch >> (level * 2);
	memcpy(pdwDest, &m_bCompVector[level][0], lPitch);
}

static WORD GetNumberOfBits(DWORD dwMask)
{
	WORD wBits;
    for (wBits = 0; dwMask; wBits++)
        dwMask = (dwMask & (dwMask - 1));

    return wBits;
}
VOID CDXTCImage::DecodePixelFormat(CHAR* strPixelFormat, XDDPIXELFORMAT* pxddpf)
{
	switch (pxddpf->dwFourCC)
	{
		case 0:
			{

				WORD a = GetNumberOfBits(pxddpf->dwRGBAlphaBitMask);
				WORD r = GetNumberOfBits(pxddpf->dwRBitMask);
				WORD g = GetNumberOfBits(pxddpf->dwGBitMask);
				WORD b = GetNumberOfBits(pxddpf->dwBBitMask);

				_snprintf(strPixelFormat, 31, "ARGB-%d%d%d%d%s", a, r, g, b,
					pxddpf->dwBBitMask & DDPF_ALPHAPREMULT ? "-premul" : "");
				m_CompFormat = PF_ARGB;
			}
			break;

		case MAKEFOURCC('D','X','T','1'):
			strncpy(strPixelFormat, "DXT1", 31);
			m_CompFormat = PF_DXT1;
			break;

		case MAKEFOURCC('D','X','T','2'):
			strncpy(strPixelFormat, "DXT2", 31);
			m_CompFormat = PF_DXT2;
			break;

		case MAKEFOURCC('D','X','T','3'):
			strncpy(strPixelFormat, "DXT3", 31);
			m_CompFormat = PF_DXT3;
			break;

		case MAKEFOURCC('D','X','T','4'):
			strncpy(strPixelFormat, "DXT4", 31);
			m_CompFormat = PF_DXT4;
			break;

		case MAKEFOURCC('D','X','T','5'):
			strncpy(strPixelFormat, "DXT5", 31);
			m_CompFormat = PF_DXT5;
			break;

		default:
			strcpy(strPixelFormat, "Format Unknown");
			m_CompFormat = PF_UNKNOWN;
			break;
	}
}

