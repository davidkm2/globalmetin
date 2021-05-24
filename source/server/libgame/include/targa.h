#ifndef __INC_SERVER_TARGA_H__
#define __INC_SERVER_TARGA_H__

#pragma pack(1)

struct TGA_HEADER
{
    char idLen;		// 0
    char palType;	// �ķ�Ʈ������ 1, ���� 0
    char imgType;	// �ķ�Ʈ������ 1, ���� 2
    uint16_t colorBegin;	// 0
    uint16_t colorCount;	// �ķ�Ʈ ������ 256, ���� 0
    char palEntrySize;	// �ķ�Ʈ ������ 24, ���� 0
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
    char colorBits;
    char desc;
};

#define IMAGEDESC_ORIGIN_MASK	0x30
#define IMAGEDESC_TOPLEFT	0x20
#define IMAGEDESC_BOTLEFT	0x00
#define IMAGEDESC_BOTRIGHT	0x10
#define IMAGEDESC_TOPRIGHT	0x30

#pragma pack()

class CTargaImage
{
	public:
		CTargaImage();
		~CTargaImage();

		void	Create(int32_t x, int32_t y);
		char *	GetBasePointer(int32_t line = 0);
		bool	Save(const char * filename);

	protected:
		TGA_HEADER	m_header;
		char *		m_pbuf;
		int32_t		m_x, m_y;
};

#endif
