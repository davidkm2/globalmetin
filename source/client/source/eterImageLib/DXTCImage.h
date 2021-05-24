#ifndef AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_
#define AFX_IMAGE_DXTC_H__4B89D8D0_7857_11D4_9630_00A0C996DE3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include <vector>

enum EPixFormat
{
	PF_ARGB,
	PF_DXT1,
	PF_DXT2,
	PF_DXT3,
	PF_DXT4,
	PF_DXT5,
	PF_UNKNOWN
};

#define MAX_MIPLEVELS 12

#ifndef DUMMYUNIONNAMEN
#if defined(__cplusplus) || !defined(NONAMELESSUNION)
#define DUMMYUNIONNAMEN(n)
#else
#define DUMMYUNIONNAMEN(n)      u##n
#endif
#endif

typedef struct _XDDPIXELFORMAT
{
	DWORD       dwSize;                 
	DWORD       dwFlags;                
	DWORD       dwFourCC;               

    union
    {
        DWORD   dwRGBBitCount;          
        DWORD   dwYUVBitCount;          
        DWORD   dwZBufferBitDepth;      
        DWORD   dwAlphaBitDepth;        
        DWORD   dwLuminanceBitCount;    
        DWORD   dwBumpBitCount;         
        DWORD   dwPrivateFormatBitCount;

    } DUMMYUNIONNAMEN(1);
    union
    {
        DWORD   dwRBitMask;             
        DWORD   dwYBitMask;             
        DWORD   dwStencilBitDepth;      
        DWORD   dwLuminanceBitMask;     
        DWORD   dwBumpDuBitMask;        
        DWORD   dwOperations;           
    } DUMMYUNIONNAMEN(2);
    union
    {
        DWORD   dwGBitMask;             
        DWORD   dwUBitMask;             
        DWORD   dwZBitMask;             
        DWORD   dwBumpDvBitMask;        
        struct
        {
            WORD    wFlipMSTypes;       
            WORD    wBltMSTypes;        
        } MultiSampleCaps;

    } DUMMYUNIONNAMEN(3);
    union
    {
        DWORD   dwBBitMask;             
        DWORD   dwVBitMask;             
        DWORD   dwStencilBitMask;       
        DWORD   dwBumpLuminanceBitMask; 
    } DUMMYUNIONNAMEN(4);
    union
    {
        DWORD   dwRGBAlphaBitMask;      
        DWORD   dwYUVAlphaBitMask;      
        DWORD   dwLuminanceAlphaBitMask;
        DWORD   dwRGBZBitMask;          
        DWORD   dwYUVZBitMask;          
    } DUMMYUNIONNAMEN(5);
} XDDPIXELFORMAT;
class CDXTCImage
{
	public:
		CDXTCImage();
		virtual ~CDXTCImage();

		void	Initialize();
		void	Clear();

	public:
		const BYTE *		m_pbCompBufferByLevels[MAX_MIPLEVELS];
		std::vector<BYTE>	m_bCompVector[MAX_MIPLEVELS];

		int					m_nCompSize;
		int					m_nCompLineSz;

		char				m_strFormat[32];
		EPixFormat			m_CompFormat;

		long				m_lPitch;
		DWORD				m_dwMipMapCount;
		bool				m_bMipTexture;
		DWORD				m_dwFlags;

		int					m_nWidth;
		int					m_nHeight;

		XDDPIXELFORMAT		m_xddPixelFormat;

		bool LoadFromFile(const char * filename);
		bool LoadFromMemory(const BYTE * c_pbMap);
		bool LoadHeaderFromMemory(const BYTE * c_pbMap);
		bool Copy(int miplevel, BYTE * pbDest, long lDestPitch);

		void Decompress(int miplevel, DWORD * pdwDest);
		void DecompressDXT1(int miplevel, DWORD * pdwDest);
		void DecompressDXT3(int miplevel, DWORD * pdwDest);
		void DecompressDXT5(int miplevel, DWORD * pdwDest);
		void DecompressARGB(int miplevel, DWORD * pdwDest);

		VOID DecodePixelFormat(CHAR* strPixelFormat, XDDPIXELFORMAT* pddpf);

		void Unextract(BYTE * pbDest, int iWidth, int iHeight, int iPitch);
};

#endif 

