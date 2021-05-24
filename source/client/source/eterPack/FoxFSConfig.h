#ifndef FOXFS_CONFIG_H
#define FOXFS_CONFIG_H

#define FOXFS_MAGIC (0xFFFFFFF)
#define FOXFS_LZ4_BLOCKSIZE (0x00001000)

namespace FoxFS
{
	
#pragma pack(push, 1)
	typedef struct SArchiveHeader
	{
		uint32_t magic;
		unsigned char key[32];
		unsigned char iv[32];
	} TArchiveHeader;
	typedef struct SArchiveEntry
	{
		uint32_t decompressed;
		uint32_t hash;
		uint32_t offset;
		uint32_t size;
		uint32_t name;
	} TArchiveEntry;
#pragma pack(pop)
	
}

#endif