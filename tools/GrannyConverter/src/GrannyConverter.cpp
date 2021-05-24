#include "../../../source/client/eterBase/Filename.h"
#include "../../../source/client/eterBase/CRC32.h"
#include <granny2/granny2.4.0.10.h>

#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4189)
#pragma warning(disable:4512)
#pragma warning(disable:4706)
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <cryptopp/tiger.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/des.h>
#include <cryptopp/aes.h>
#include <cryptopp/tea.h>
#include <cryptopp/osrng.h>
#include <cryptopp/salsa.h>
#include <cryptopp/panama.h>
#include <cryptopp/sosemanuk.h>
#pragma warning(pop)
#include <boost/timer.hpp>

#pragma comment(lib, "granny2.4.0.10.lib")
#ifdef _DEBUG
#pragma comment(lib, "cryptlib_debug.lib")
#else
#pragma comment(lib, "cryptlib_release.lib")
#endif

struct CryptoTester
{
	CryptoTester() : Encryptor(NULL), Decryptor(NULL), elapsedTime(0.0) {}
	CryptoPP::StreamTransformation* Encryptor;
	CryptoPP::StreamTransformation* Decryptor;
	double elapsedTime;
};

template <class N> void SetupCrypto(std::vector<CryptoTester>& vector, const byte* key, const byte* iv)
{
	CryptoTester res;

	res.Encryptor = new N::Encryption;
	res.Decryptor = new N::Decryption;

	CryptoPP::SimpleKeyingInterface* keyInterface = dynamic_cast<CryptoPP::SimpleKeyingInterface*>(res.Encryptor);
	CryptoPP::SimpleKeyingInterface* keyInterface2 = dynamic_cast<CryptoPP::SimpleKeyingInterface*>(res.Decryptor);

	if (keyInterface->IVRequirement() != CryptoPP::SimpleKeyingInterface::NOT_RESYNCHRONIZABLE)
	{
		keyInterface->SetKeyWithIV(key, keyInterface->MaxKeyLength(), iv, keyInterface->MaxIVLength());
		keyInterface2->SetKeyWithIV(key, keyInterface->MaxKeyLength(), iv, keyInterface->MaxIVLength());
	}
	else
	{
		keyInterface->SetKey(key, keyInterface->MaxKeyLength());
		keyInterface2->SetKey(key, keyInterface->MaxKeyLength());
	}

	vector.push_back(res);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
		return 1;

	char arg1[1024 + 1];
	strcpy_s(arg1, sizeof(arg1), argv[1]);

	CFilename Gr2FileName(arg1);
	
	CFilename RawFileName(Gr2FileName.NoExtension());
	RawFileName += CFilename(".ygr");

	CFilename NewGr2FileName(Gr2FileName.NoExtension());
	NewGr2FileName += CFilename("_new.gr2");

	granny_file* gr2File = GrannyReadEntireFile(Gr2FileName.c_str());
	granny_int32x FileSectionCount;
	granny_file_info * FileInfo;

	if (gr2File)
	{
		FileSectionCount = gr2File->SectionCount;
		FileInfo = GrannyGetFileInfo(gr2File);

		GrannyConvertFileInfoToRaw(FileInfo, RawFileName.c_str());
	}

	FILE * fp;
	
	if (0 == fopen_s(&fp, RawFileName.c_str(), "rb") && fp)
	{
		fseek(fp, 0L, SEEK_END);
		long FileSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		char * Buffer = (char *) malloc(FileSize);
		printf("FileSize: %u\n", fread(Buffer, sizeof(char), FileSize, fp));
		fclose(fp);

		// ���� �̸����� ���� ��ȣȭ Ű�� ������ (�ٸ� ���Ϸ� �̸� �ٲ㼭 ����ϱ� �������)
		std::string SrcStringForKey(RawFileName);
		CryptoPP::HashTransformation* hm = NULL;
		unsigned int idx = GetCRC32(SrcStringForKey.c_str(), SrcStringForKey.length()) & 3;

		// Ű ��ȣȭ
		byte key[32];

		CryptoPP::HashTransformation* hm1 = NULL;
		CryptoPP::HashTransformation* hm2 = NULL;

		static CryptoPP::Tiger tiger;
		static CryptoPP::SHA1 sha1;
		static CryptoPP::RIPEMD128 ripemd128;
		static CryptoPP::Whirlpool whirlpool;

		switch (idx & 3)
		{
			case 0:
				hm1 = &whirlpool;
				break;

			case 1:
				hm1 = &tiger;
				break;

			case 2:
				hm1 = &sha1;
				break;

			case 3:
				hm1 = &ripemd128;
				break;
		}

		CryptoPP::StringSource(SrcStringForKey, true,
						 new CryptoPP::HashFilter(*hm1, 
							//new CryptoPP::HexEncoder(
								new CryptoPP::ArraySink(key, sizeof(key) - 16)
							//) // HexEncoder
						 ) // HashFilter
					 ); // StringSource

		// ������� Ű�� ù��° 4����Ʈ�� ���� 16����Ʈ Ű ���� �˰��� ����
		unsigned int idx2 = *(unsigned int*) key;

		switch (idx2 & 3)
		{
			case 0:
				hm2 = &sha1;
				break;

			case 1:
				hm2 = &ripemd128;
				break;

			case 2:
				hm2 = &whirlpool;
				break;

			case 3:
				hm2 = &tiger;
				break;
		}

		CryptoPP::StringSource(SrcStringForKey, true,
						 new CryptoPP::HashFilter(*hm2, 
							//new CryptoPP::HexEncoder(
								new CryptoPP::ArraySink(key + 16, sizeof(key) - 16)
							//) // HexEncoder
						 ) // HashFilter
					 ); // StringSource
		// Ű ���� �Ϸ�

		// IV ����
		CryptoPP::AutoSeededRandomPool prng;
		byte iv[32];
		prng.GenerateBlock(iv, sizeof(iv));

		// Crypto�� �¾�
		std::vector<CryptoTester> tester;

		SetupCrypto<CryptoPP::ECB_Mode<CryptoPP::AES> >(tester, key, iv);
		SetupCrypto<CryptoPP::CBC_Mode<CryptoPP::AES> >(tester, key, iv);
		SetupCrypto<CryptoPP::PanamaCipher<CryptoPP::LittleEndian> >(tester, key, iv);
		SetupCrypto<CryptoPP::Salsa20>(tester, key, iv);
		SetupCrypto<CryptoPP::XSalsa20>(tester, key, iv);
		SetupCrypto<CryptoPP::Sosemanuk>(tester, key, iv);
		SetupCrypto<CryptoPP::CBC_Mode<CryptoPP::XTEA> >(tester, key, iv);
		SetupCrypto<CryptoPP::ECB_Mode<CryptoPP::XTEA> >(tester, key, iv);

		std::string CipherText;  // ��ȣȭ�� ���� �ӽ� ����
		std::string DecryptText;  // ��ȣȭ�� ���� �ӽ� ����

		double nonono = 0.0;

		for (int i = 0; i < 1024; ++i)
		{
			CipherText.clear();
			DecryptText.clear();

			std::vector<CryptoTester>::iterator it = tester.begin();

			while (it != tester.end())
			{
				CryptoTester& tester = *(it++);

				boost::timer t;

				CryptoPP::ArraySource((const byte*) Buffer, FileSize, true, 
					new CryptoPP::StreamTransformationFilter(*tester.Encryptor, 
						new CryptoPP::StringSink(CipherText)
					)
				);

				CryptoPP::ArraySource(CipherText, true,
					new CryptoPP::StreamTransformationFilter(*tester.Decryptor,
						new CryptoPP::StringSink(DecryptText)
					)
				);

				tester.elapsedTime += t.elapsed();
			}

			boost::timer t;

			for (int k = 0; k < FileSize >> 5; k += 32)
				CipherText.replace(k, 32, Buffer + k, 32);

			const char * text = CipherText.c_str();

			for (int k = 0; k < FileSize >> 5; k += 32)
				DecryptText.replace(k, 32, text + k, 32);

			nonono += t.elapsed();
		}

		std::vector<CryptoTester>::iterator it = tester.begin();

		while (it != tester.end())
		{
			CryptoTester& tester = *(it++);
			printf("%s %g\n", tester.Encryptor->AlgorithmName().c_str(), tester.elapsedTime);
		}

		printf("No Crypto: %g\n", nonono);

		// �ٽ� �׷��� ���Ϸ� ����
		FileInfo = (granny_file_info *)DecryptText.c_str();

		GrannyRebasePointers(GrannyFileInfoType, (void*) FileInfo, (int) FileInfo, true);

		granny_int32x DefaultSectionIndex = 0;

		int artToolSize = GrannyGetTotalObjectSize(GrannyArtToolInfoType);

		granny_file_data_tree_writer *DataTreeWriter =
		   GrannyBeginFileDataTreeWriting(GrannyFileInfoType, FileInfo,
									 DefaultSectionIndex,
									 DefaultSectionIndex);

		GrannyWriteDataTreeToFile(DataTreeWriter, GrannyCurrentGRNStandardTag,
							 NewGr2FileName.c_str(),
							 FileSectionCount);

		GrannyEndFileDataTreeWriting(DataTreeWriter);

		free(Buffer);
	}

	return 0;
}
