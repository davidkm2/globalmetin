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

CAttribute::CAttribute(uint32_t width, uint32_t height) // dword Ÿ������ ��� 0�� ä���.
{
    Initialize(width, height);
    Alloc();
}

CAttribute::CAttribute(uint32_t * attr, uint32_t width, uint32_t height) // attr�� �о smart�ϰ� �Ӽ��� �о�´�.
{
    Initialize(width, height);

    int32_t i;
    int32_t size = width * height;

    for (i = 0; i < size; ++i)
	if (attr[0] != attr[i])
	    break;

    // �Ӽ��� ���� ������ ���� defaultAttr�� �����Ѵ�.
    if (i == size)
	defaultAttr = attr[0];
    else
    {
	int32_t allAttr = 0;

	for (i = 0; i < size; ++i)
	    allAttr |= attr[i];

	// ���� 8��Ʈ�� ����� ��� D_BYTE
	if (!(allAttr & 0xffffff00))
	    dataType = D_BYTE;
	// ���� 16��Ʈ�� ����� ��� D_WORD
	else if (!(allAttr & 0xffff0000))
	    dataType = D_WORD;
	else // �� �̿ܿ��� D_DWORD
	    dataType = D_DWORD;

	Alloc();

	if (dataType == D_DWORD) // D_DWORD�� ���� ���� �Ӽ��� �����Ƿ� ���� ����.
	    memcpy(data, attr, sizeof(uint32_t) * width * height);
	else
	{
	    // �ƴϸ� ����Ʈ �ؾ� �Ѵ�.
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

    if (!data) // �Ӽ��� ������ �� ���� �����Ͱ� ������ �׳� �����Ѵ�.
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

