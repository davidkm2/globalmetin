#include "../../libthecore/include/stdafx.h"
#include "attribute.h"

#define SET_BIT(var,bit)                ((var) |= (bit))
#define REMOVE_BIT(var,bit)             ((var) &= ~(bit))

void CAttribute::Initialize(uint32_t width_, uint32_t height_)
{
    dataType = D_DWORD;
    defaultAttr = 0;
    width = width_;
    height = height_;
    data = NULL;
    bytePtr = NULL;
    wordPtr = NULL;
    dwordPtr = NULL;
}

void CAttribute::Alloc()
{
    size_t memSize;

    switch (dataType)
    {
	case D_DWORD:
	    memSize = width * height * sizeof(uint32_t);
	    break;

	case D_WORD:
	    memSize = width * height * sizeof(uint16_t);
	    break;

	case D_BYTE:
	    memSize = width * height;
	    break;

	default:
	    assert(!"dataType error!");
	    return;
    }

    //sys_log(0, "Alloc::dataType %u width %d height %d memSize %d", dataType, width, height, memSize);
    data = malloc(memSize);

    switch (dataType)
    {
	case D_DWORD:
	    dwordPtr = new uint32_t * [height];
	    dwordPtr[0] = (uint32_t *) data;

	    for (uint32_t y = 1; y < height; ++y)
		dwordPtr[y] = dwordPtr[y - 1] + width;

	    for (uint32_t y = 0; y < height; ++y)
		for (uint32_t x = 0; x < width; ++x)
		    dwordPtr[y][x] = defaultAttr;

	    break;

	case D_WORD:
	    wordPtr = new uint16_t * [height];
	    wordPtr[0] = (uint16_t *) data;

	    for (uint32_t y = 1; y < height; ++y)
		wordPtr[y] = wordPtr[y - 1] + width;

	    for (uint32_t y = 0; y < height; ++y)
		for (uint32_t x = 0; x < width; ++x)
					wordPtr[y][x] = (uint16_t)defaultAttr;

	    break;

	case D_BYTE:
	    bytePtr = new uint8_t * [height];
	    bytePtr[0] = (uint8_t *) data;

	    for (uint32_t y = 1; y < height; ++y)
		bytePtr[y] = bytePtr[y - 1] + width;

	    for (uint32_t y = 0; y < height; ++y)
		for (uint32_t x = 0; x < width; ++x)
					bytePtr[y][x] = (uint8_t)defaultAttr;

	    break;
    }
}

CAttribute::CAttribute(uint32_t width, uint32_t height) // dword 타잎으로 모두 0을 채운다.
{
    Initialize(width, height);
    Alloc();
}

CAttribute::CAttribute(uint32_t * attr, uint32_t width, uint32_t height) // attr을 읽어서 smart하게 속성을 읽어온다.
{
    Initialize(width, height);

    int32_t i;
    int32_t size = width * height;

    for (i = 0; i < size; ++i)
	if (attr[0] != attr[i])
	    break;

    // 속성이 전부 같으면 단지 defaultAttr만 설정한다.
    if (i == size)
	defaultAttr = attr[0];
    else
    {
	int32_t allAttr = 0;

	for (i = 0; i < size; ++i)
	    allAttr |= attr[i];

	// 하위 8비트만 사용할 경우 D_BYTE
	if (!(allAttr & 0xffffff00))
	    dataType = D_BYTE;
	// 하위 16비트만 사용할 경우 D_WORD
	else if (!(allAttr & 0xffff0000))
	    dataType = D_WORD;
	else // 그 이외에는 D_DWORD
	    dataType = D_DWORD;

	Alloc();

	if (dataType == D_DWORD) // D_DWORD일 때는 원본 속성과 같으므로 단지 복사.
	    memcpy(data, attr, sizeof(uint32_t) * width * height);
	else
	{
	    // 아니면 컨버트 해야 한다.
	    uint32_t * pdw = (uint32_t *) attr;

	    if (dataType == D_BYTE)
	    {
		for (uint32_t y = 0; y < height; ++y)
		    for (uint32_t x = 0; x < width; ++x)
						bytePtr[y][x] = (uint8_t)(*(pdw++));
	    }
	    else if (dataType == D_WORD)
	    {
		for (uint32_t y = 0; y < height; ++y)
		    for (uint32_t x = 0; x < width; ++x)
						wordPtr[y][x] = (uint16_t)(*(pdw++));
	    }
	}
    }
}

CAttribute::~CAttribute()
{
    if (data)
	free(data);

    if (bytePtr)
	delete [] bytePtr;

    if (wordPtr)
	delete [] wordPtr;

    if (dwordPtr)
	delete [] dwordPtr;
}

int32_t CAttribute::GetDataType()
{
    return dataType;
}

void * CAttribute::GetDataPtr()
{
    return data;
}

void CAttribute::Set(uint32_t x, uint32_t y, uint32_t attr)
{
    if (x > width || y > height)
	return;

    if (!data)
	Alloc();

    if (bytePtr)
    {
	SET_BIT(bytePtr[y][x], attr);
	return;
    }

    if (wordPtr)
    {
	SET_BIT(wordPtr[y][x], attr);
	return;
    }

    SET_BIT(dwordPtr[y][x], attr);
}

void CAttribute::Remove(uint32_t x, uint32_t y, uint32_t attr)
{
    if (x > width || y > height)
	return;

    if (!data) // 속성을 삭제할 때 만약 데이터가 없으면 그냥 리턴한다.
	return;

    if (bytePtr)
    {
	REMOVE_BIT(bytePtr[y][x], attr);
	return;
    }

    if (wordPtr)
    {
	REMOVE_BIT(wordPtr[y][x], attr);
	return;
    }

    REMOVE_BIT(dwordPtr[y][x], attr);
}

uint32_t CAttribute::Get(uint32_t x, uint32_t y)
{
    if (x > width || y > height)
	return 0;

    if (!data)
	return defaultAttr;

    if (bytePtr)
	return bytePtr[y][x];

    if (wordPtr)
	return wordPtr[y][x];

    return dwordPtr[y][x];
}

void CAttribute::CopyRow(uint32_t y, uint32_t * row)
{
    if (!data)
    {
	for (uint32_t x = 0; x < width; ++x)
	    row[x] = defaultAttr;

	return;
    }

    if (dwordPtr)
	memcpy(row, dwordPtr[y], sizeof(uint32_t) * width);
    else
    {
	if (bytePtr)
	{
	    for (uint32_t x = 0; x < width; ++x)
		row[x] = bytePtr[y][x];
	}
	else if (wordPtr)
	{
	    for (uint32_t x = 0; x < width; ++x)
		row[x] = wordPtr[y][x];
	}
    }
}

