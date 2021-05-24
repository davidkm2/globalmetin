#ifndef __INC_ATTRIBUTE_H__
#define __INC_ATTRIBUTE_H__

enum EDataType
{
    D_DWORD,
    D_WORD,
    D_BYTE
};

//
// 맵 속성들을 처리할 때 사용
//
class CAttribute
{
    public:
	CAttribute(uint32_t width, uint32_t height); // dword 타잎으로 모두 0을 채운다.
	CAttribute(uint32_t * attr, uint32_t width, uint32_t height); // attr을 읽어서 smart하게 속성을 읽어온다.
	~CAttribute();
	void Alloc();
	int32_t GetDataType();
	void * GetDataPtr();
	void Set(uint32_t x, uint32_t y, uint32_t attr);
	void Remove(uint32_t x, uint32_t y, uint32_t attr);
	uint32_t Get(uint32_t x, uint32_t y);
	void CopyRow(uint32_t y, uint32_t * row);

    private:
	void Initialize(uint32_t width, uint32_t height);

    private:
	int32_t dataType;
	uint32_t defaultAttr;
	uint32_t width, height;

	void * data;

	uint8_t **	bytePtr;
	uint16_t **	wordPtr;
	uint32_t ** dwordPtr;
};

#endif
