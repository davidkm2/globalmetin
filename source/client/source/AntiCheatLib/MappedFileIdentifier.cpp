#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../eterPack/EterPackManager.h"

#include <XOrstr.h>
#include <fstream>

std::string GetMappedFileHash(const std::string & strFileName)
{
	std::string strHash = "";

	if (strFileName.empty())
		return strHash;

#ifdef ENABLE_FOX_FS
	if (CFileSystem::Instance().isExistInPack(strFileName.c_str(), __FUNCTION__) == false)
#else
	if (CEterPackManager::Instance().isExistInPack(strFileName.c_str()) == false)
#endif
		return strHash;

	CMappedFile lpMpFile;
	const void * pData = nullptr;
#ifdef ENABLE_FOX_FS
	if (CFileSystem::Instance().GetFromPack(lpMpFile, strFileName.c_str(), &pData) == false)
#else
	if (CEterPackManager::Instance().GetFromPack(lpMpFile, strFileName.c_str(), &pData) == false)
#endif
		return strHash;

	if (!lpMpFile.Get())
		return strHash;

	strHash = CAnticheatManager::Instance().GetMd5((const uint8_t*)lpMpFile.Get(), lpMpFile.Size());

	return strHash;
}

static struct stCheckFileList {
	const char*			c_szFileName;
	const char*			c_szFileMd5;
} st_FileHashList[] = {
	//	 filename								    md5

	{ "d:\\ymir work\\pc\\warrior\\horse\\wait.msa",		"0ca623f31bb662c795e30f76dcac761c" }, // 0
	{ "d:\\ymir work\\pc\\warrior\\horse\\damage.msa",		"87fba70b9c39e55256c9bd8fefd24daa" }, // 1
	{ "d:\\ymir work\\pc\\warrior\\horse\\wait_1.msa",		"24dfcdca31d9df539075ee70e375f9fd" }, // 2
	{ "d:\\ymir work\\pc\\warrior\\horse\\wait_2.msa",		"1411f53f4a1afd48bf9ff2829f2f2422" }, // 3
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_01.msa",		"2da4eabbad6b135d58376964cc28d232" }, // 4
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_02.msa",		"a8d91cbdfd240ea663b65eff927ab268" }, // 5
	{ "d:\\ymir work\\pc\\warrior\\horse_onehand_sword\\combo_03.msa",		"d2e579ab6d07d77513f5cebc06db3efd" }, // 6
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_01.msa",		"a396a7adf6ada434541d1c902bb441a6" }, // 7
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_02.msa",		"44aed62a36f4e8f49d5d192c81eded91" }, // 8
	{ "d:\\ymir work\\pc\\warrior\\horse_twohand_sword\\combo_03.msa",		"2d1fe846f2424ae305e933f39975c197" }, // 9
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_01.msa",		"d5157530faf7a7a62dbe3305e33d7901" }, // 10
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_02.msa",		"3fc937d2ec1b1bb1fccb2e5436265a43" }, // 11
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_03.msa",		"5b770381ea510dc5e9cc29568559773a" }, // 12
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_04.msa",		"d270c27a30897c6f7616c39640823cd1" }, // 13
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_05.msa",		"788c9e55d9f071efd9bbebc8e41b3a7c" }, // 14
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_06.msa",		"44ba15627638829ab8037063c6610aeb" }, // 15
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\combo_07.msa",		"16e493a84ee8f2baf98c76374063cb7a" }, // 16
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage.msa",		"e0045a196e8ad146deb258b497a428c3" }, // 17
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_1.msa",		"699e31e825807db87729a27dfdf20595" }, // 18
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_2.msa",		"d8a8d4d56d0aec535d43169af4450240" }, // 19
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\damage_3.msa",		"c343b40fb5594d8a9c98346a5e93bcd4" }, // 20
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\wait.msa",		"3c2af5d10675b5226aa9ed6d3f6545b3" }, // 21
	{ "d:\\ymir work\\pc\\warrior\\onehand_sword\\wait_1.msa",		"2de7f5289caff1b94ba5d502346e1147" }, // 22
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_01.msa",		"f36abeffe48b8ea28914811bd4b64f18" }, // 23
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_02.msa",		"bb1aaa77b804167e7000fc9ad409f527" }, // 24
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_03.msa",		"839bb72a140c7a65b3e9b6443c145521" }, // 25
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_04.msa",		"789179dea92aadb4a507ca316f136f08" }, // 26
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_05.msa",		"9bdaf95b42814659434cd085ca76c6af" }, // 27
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_06.msa",		"2f9999a679d6369758e7115bb3bc7409" }, // 28
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\combo_07.msa",		"08e4430e4c5b1baaf2c5941163af2d89" }, // 29
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage.msa",		"82846659be5aee19fb0856aba9eb76e8" }, // 30
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_1.msa",		"37286497efb18312494fa677eba2dae6" }, // 31
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_2.msa",		"1a3f94d699fc1d2a153cc45760afe683" }, // 32
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\damage_3.msa",		"e537ae7129b969e9724aa50cadf69300" }, // 33
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\wait.msa",		"94b5e6770f2064dfe4de9a95ba39813a" }, // 34
	{ "d:\\ymir work\\pc\\warrior\\twohand_sword\\wait_1.msa",		"1d6296e6409bdc18ce5026648ba412d8" }, // 35
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait.msa",		"16778aff5ff09643fbebbc22eb86c6f4" }, // 36
	{ "d:\\ymir work\\pc2\\warrior\\horse\\damage.msa",		"beeda94a120f910f3d3010c10375565f" }, // 37
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait_1.msa",		"6e321f5231c494709bc9f98f1e12d8fe" }, // 38
	{ "d:\\ymir work\\pc2\\warrior\\horse\\wait_2.msa",		"d093c07ef088453ed134f7e31902e75f" }, // 39
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_01.msa",		"c890fbdd915e871755bfaff87b5f67c8" }, // 40
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_02.msa",		"8ba16a0400a0a322b5512ec75471ca45" }, // 41
	{ "d:\\ymir work\\pc2\\warrior\\horse_onehand_sword\\combo_03.msa",		"166974b69f818a10231420fa663dbf05" }, // 42
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_01.msa",		"c890fbdd915e871755bfaff87b5f67c8" }, // 43
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_02.msa",		"8ba16a0400a0a322b5512ec75471ca45" }, // 44
	{ "d:\\ymir work\\pc2\\warrior\\horse_twohand_sword\\combo_03.msa",		"166974b69f818a10231420fa663dbf05" }, // 45
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_01.msa",		"b4f2f87dcb0ee872502a451eabbdd086" }, // 46
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_02.msa",		"be7bbae448d4a62a9fd1fcf642580727" }, // 47
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_03.msa",		"cdb2e9c6d5efa621211a2bf7ef7c1f67" }, // 48
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_04.msa",		"da5711d76b339c03ba7a3a5811f46733" }, // 49
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_05.msa",		"b202f808205cedcb47b6d785a35b1227" }, // 50
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_06.msa",		"18c1c5c34011ef07b7732ce568f4d439" }, // 51
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\combo_07.msa",		"5738278fc72a9c6714d76377dc48175d" }, // 52
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage.msa",		"fe56aeba3cb3b5aa1d4ce6af75626c52" }, // 53
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_1.msa",		"1311492d9afd90fc7944a1d89eca2ffd" }, // 54
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_2.msa",		"53448c011f4c9a1eff2eacd335d6fa6b" }, // 55
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\damage_3.msa",		"d92a48a931dd61022cf93368d7e19d54" }, // 56
	{ "d:\\ymir work\\pc2\\warrior\\onehand_sword\\wait.msa",		"5136dfd0a90be2b8a00190d20b659b41" }, // 57
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_01.msa",		"233cb85c0dbd23020f804a065af7d26d" }, // 58
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_02.msa",		"ae29d852563ac302d0bc4a871de1850a" }, // 59
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_03.msa",		"62d2e7bcb5f30e7444a993a29c537ebf" }, // 60
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_04.msa",		"8b332510db6e80137069a2145c71b433" }, // 61
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_05.msa",		"4852e2f8407ce850d62d93f9d0a31797" }, // 62
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_06.msa",		"567a9621eff569b1ff5ac46dea5727e1" }, // 63
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\combo_07.msa",		"5a4f2a2a42bb81e2d1eb654167862dae" }, // 64
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage.msa",		"1dbf1a0ec6dae0b9eed828c019028415" }, // 65
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_1.msa",		"d3446aae7b110f945488c8e680381cfe" }, // 66
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_2.msa",		"5c44f19f91633c87395bf6072f8f67f1" }, // 67
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\damage_3.msa",		"c6c9fc0a27872f02f5eb5cd629220063" }, // 68
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\wait.msa",		"73b2273118785801b5e9137ade4b0a9b" }, // 69
	{ "d:\\ymir work\\pc2\\warrior\\twohand_sword\\wait_1.msa",		"2155db1b9ce6d4b7af4f4cbc9d07a89f" }, // 70
	{ "d:\\ymir work\\pc\\sura\\horse\\wait.msa",		"2b0d37fd55e2fb2acd5d1dbf465133c8" }, // 71
	{ "d:\\ymir work\\pc\\sura\\horse\\damage.msa",		"35b1026d8c540ce9e8edf993bb2994f0" }, // 72
	{ "d:\\ymir work\\pc\\sura\\horse\\wait_1.msa",		"51401340b97eb31a6e498ab9d1262552" }, // 73
	{ "d:\\ymir work\\pc\\sura\\horse\\wait_2.msa",		"422b721045995199e6b20d28fcd0566e" }, // 74
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_01.msa",		"9a5adbbca72a36b33cfb6c9a863601a9" }, // 75
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_02.msa",		"8c9c03aa5c3ca131c3098f137acc48d4" }, // 76
	{ "d:\\ymir work\\pc\\sura\\horse_onehand_sword\\combo_03.msa",		"ca15426e919e5dffe6250509fad2facd" }, // 77
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_01.msa",		"df317c0a02efed237b6e3fbf4127e391" }, // 78
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_02.msa",		"aa87cfa880cfc2783640f2a4be7481d0" }, // 79
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_03.msa",		"a942f917b7e91d2fc9372bfa9501fb57" }, // 80
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_05.msa",		"ee93b1531d6c10ce7912f2965d3cf30b" }, // 81
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_06.msa",		"d0da6073ae42f9eb4a0770da13a9e1cd" }, // 82
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\combo_07.msa",		"4f8d5f50141a76d8ca867ebf8e6e646f" }, // 83
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage.msa",		"5c2eeadc59f827aff4ca1b6e40e1cef9" }, // 84
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_1.msa",		"954009ab7aab6fc6453dbf84212e7c8c" }, // 85
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_2.msa",		"9d90289df446a4108beaf14317475989" }, // 86
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\damage_3.msa",		"77a03999132991bf29533fabd42e97e7" }, // 87
	{ "d:\\ymir work\\pc\\sura\\onehand_sword\\wait.msa",		"1dc1411f3cd9340d261ace45f1677d95" }, // 88
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait.msa",		"80e7437176e3e38d553df9d8eb0fa332" }, // 89
	{ "d:\\ymir work\\pc2\\sura\\horse\\damage.msa",		"692c9fbcf5346f30b692112a7f2fbe4f" }, // 90
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait_1.msa",		"76c93c9b623adfc899c3a41b3521b0b9" }, // 91
	{ "d:\\ymir work\\pc2\\sura\\horse\\wait_2.msa",		"f2b73a2d725caf5f1efa116ab20679f7" }, // 92
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_01.msa",		"b43c70b3773a6324a5e5d7b3fef37128" }, // 93
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_02.msa",		"f987156931f48e5c334083dc0c92cd58" }, // 94
	{ "d:\\ymir work\\pc2\\sura\\horse_onehand_sword\\combo_03.msa",		"bca22009b68f322c439faec2fb364b99" }, // 95
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_01.msa",		"8e43dd66dc6a4bdca5606b3eded0966f" }, // 96
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_02.msa",		"452d63b47dcae13002d5ee6b1f51cb03" }, // 97
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_03.msa",		"412f193a5a53ad76f911632f37ab8d5e" }, // 98
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_05.msa",		"7a6f9d39e7dcb99898b15991c3f029e5" }, // 99
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_06.msa",		"afbdaf93fa1eebf98b4c6b8684c221d7" }, // 100
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\combo_07.msa",		"5b13e6a9d37d11994ba008a93e04fc14" }, // 101
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage.msa",		"d8ef234e871ece93422e4ee0aa7905df" }, // 102
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_1.msa",		"6c9455346f29a988c08f2692baab857f" }, // 103
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_2.msa",		"fa9a74de488ead952bfe37adc00a38d0" }, // 104
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\damage_3.msa",		"eacab3e94ae43734fb1374261c149667" }, // 105
	{ "d:\\ymir work\\pc2\\sura\\onehand_sword\\wait.msa",		"bff4169ef380de051bb2e9e223659d7e" }, // 106
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait.msa",		"27ebb6ba26e58889af2c687b0d8c1c45" }, // 107
	{ "d:\\ymir work\\pc\\shaman\\horse\\damage.msa",		"7765f15c43118741f11e5bc903813d93" }, // 108
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait_1.msa",		"17594eb9e82238295279ac37ac6d5674" }, // 109
	{ "d:\\ymir work\\pc\\shaman\\horse\\wait_2.msa",		"6a9eb02eff9780a095d1aa34a2972ae7" }, // 110
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_01.msa",		"6d8f6b97d6741a06a47dd7fe8161d43e" }, // 111
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_02.msa",		"3e80d822310f0662a3e56c2904c387eb" }, // 112
	{ "d:\\ymir work\\pc\\shaman\\horse_bell\\combo_03.msa",		"86a0c859be02e5d1bcc1d0ac1c95ce04" }, // 113
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_01.msa",		"76b5a2c37ddb73c160e359939cb47a55" }, // 114
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_02.msa",		"353a4c23a301971e3f172d7e08446709" }, // 115
	{ "d:\\ymir work\\pc\\shaman\\horse_fan\\combo_03.msa",		"59f4785c25f72d58e1e6180f1e5ea974" }, // 116
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_01.msa",		"64f344462b865a3344ce0d685cff8235" }, // 117
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_02.msa",		"a768f5b8bd0e5302d5395650f5400021" }, // 118
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_03.msa",		"079c71a4c0335846f3ca4ae4af49fe19" }, // 119
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_04.msa",		"f65b936c605d5a21b4dc82b64011564a" }, // 120
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_05.msa",		"d8ef61abe76034bc0c02813e4a1fd4cb" }, // 121
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_06.msa",		"b2a995d3588c97c9bad1414caaf3f301" }, // 122
	{ "d:\\ymir work\\pc\\shaman\\bell\\combo_07.msa",		"44c2906d0c890cdd8ca08b6a3b5e56f5" }, // 123
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage.msa",		"3e844783f33716f5b0cd284a174bb454" }, // 124
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_1.msa",		"46b91f0ea281300ce6d7616eab077cd8" }, // 125
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_2.msa",		"8a58c0918197230678e7d27fb4f32f2e" }, // 126
	{ "d:\\ymir work\\pc\\shaman\\bell\\damage_3.msa",		"bc807b7065bdc5558402121d8b564498" }, // 127
	{ "d:\\ymir work\\pc\\shaman\\bell\\wait.msa",		"54dafb99955165a86016f97d944e9480" }, // 128
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_01.msa",		"94f3da3c71754a785cceb2b823416065" }, // 129
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_02.msa",		"981728b4fd08592382bced4239d6794e" }, // 130
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_03.msa",		"472836837d763c64ef628cae88a4ff7a" }, // 131
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_04.msa",		"5fd1e8c47f4400dea637e6147b752b8a" }, // 132
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_05.msa",		"dca3b159b7b7b7dece5e2c27d70be817" }, // 133
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_06.msa",		"a0a351380fc7bbff9af2f5011af15cfb" }, // 134
	{ "d:\\ymir work\\pc\\shaman\\fan\\combo_07.msa",		"7da70b0b199b9dca1a983829cdab5af4" }, // 135
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage.msa",		"0166e6237592256968686e28cde4967a" }, // 136
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_1.msa",		"49766293ac4a2454d9d2b54f30c81ac3" }, // 137
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_2.msa",		"533ca579d7cc26cfdaa8ae1246b714ae" }, // 138
	{ "d:\\ymir work\\pc\\shaman\\fan\\damage_3.msa",		"a2d37c6f86d466dc358b53164db3920d" }, // 139
	{ "d:\\ymir work\\pc\\shaman\\fan\\wait.msa",		"937bebcef86f2ca964b7f025cf237843" }, // 140
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait.msa",		"a7fe1f9638c804e51237587d29ea015e" }, // 141
	{ "d:\\ymir work\\pc2\\shaman\\horse\\damage.msa",		"fc53f784e2beea6124f022eb2a98aaaa" }, // 142
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait_1.msa",		"244d12f9ecefa0eea627100036ee81dc" }, // 143
	{ "d:\\ymir work\\pc2\\shaman\\horse\\wait_2.msa",		"fd8ccddfc8b9d29237dc45839012aa22" }, // 144
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_01.msa",		"60ac9e1adc37627d31495117324523ce" }, // 145
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_02.msa",		"311fe2d7e7bd0b9cf58c410b313177bb" }, // 146
	{ "d:\\ymir work\\pc2\\shaman\\horse_bell\\combo_03.msa",		"2d249d798ee52062d85c702cc77729f5" }, // 147
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_01.msa",		"cfbb6000cd13d7d008884d9246691823" }, // 148
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_02.msa",		"1f253af8807c871b7ea208df054dab86" }, // 149
	{ "d:\\ymir work\\pc2\\shaman\\horse_fan\\combo_03.msa",		"c3734420ec204e1bac3dc2b94b3452a0" }, // 150
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_01.msa",		"349fa05a137a5db5a12496e6aa86f08e" }, // 151
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_02.msa",		"33b1e2dbb9ea33d4c4d000de9e7eed12" }, // 152
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_03.msa",		"1a6c17baf1ee32cad3cdcc752739e256" }, // 153
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_04.msa",		"99d0dca2b3011b3736eaf722d5d21fb0" }, // 154
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_05.msa",		"136cf92be58f70c6e74f84b4c0b7ccbe" }, // 155
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_06.msa",		"4f1feee0c3c5fdeb347a07d1b0b414d6" }, // 156
	{ "d:\\ymir work\\pc2\\shaman\\bell\\combo_07.msa",		"2130313b154e0f1580581e0371a93b9d" }, // 157
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage.msa",		"831bf91871eac39ee3050264ee8cd00c" }, // 158
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_1.msa",		"deee7bf7c69dacdc69de241930ead6a6" }, // 159
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_2.msa",		"a1e4693b796f47ba25cef21d65f0dbca" }, // 160
	{ "d:\\ymir work\\pc2\\shaman\\bell\\damage_3.msa",		"469e57fd795bbf8c427c2db365998d40" }, // 161
	{ "d:\\ymir work\\pc2\\shaman\\bell\\wait.msa",		"c4fe3c56cb903fc6427fb8ff8a9d710f" }, // 162
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_01.msa",		"38a93281e809ce7e0f0108ec00993afe" }, // 163
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_02.msa",		"1619aa1805c11c163ea970f6b30aaed7" }, // 164
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_03.msa",		"05bdf0a697b4920122f2336e1ff83d95" }, // 165
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_04.msa",		"6cc7070c3f39c2927fb16bd8f8903bb6" }, // 166
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_05.msa",		"7006282d38c5afbe8828072f2136a417" }, // 167
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_06.msa",		"0f90014d3dc240963e26ac1624810cc6" }, // 168
	{ "d:\\ymir work\\pc2\\shaman\\fan\\combo_07.msa",		"1d270ab4403260a0b389b6b8ecdb049d" }, // 169
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage.msa",		"1a65161363b63fbe2e2ab3eb331d277d" }, // 170
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_1.msa",		"55a28565dc102a71d0ee020bc9542233" }, // 171
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_2.msa",		"da00afd68ff04f26865f98ad9a8ed116" }, // 172
	{ "d:\\ymir work\\pc2\\shaman\\fan\\damage_3.msa",		"498ef042775d7c3fbe446c355952b2d1" }, // 173
	{ "d:\\ymir work\\pc2\\shaman\\fan\\wait.msa",		"d2ea05392a01357125fba1f8c78fcf14" }, // 174
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait.msa",		"f2086de51696036178459b889587deae" }, // 175
	{ "d:\\ymir work\\pc\\assassin\\horse\\damage.msa",		"85bdab5b8f816d2bc45c8aac9d5d2b62" }, // 176
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait_1.msa",		"f24889b7693f3ad052f79f920011297d" }, // 177
	{ "d:\\ymir work\\pc\\assassin\\horse\\wait_2.msa",		"93b31c58c57379746631f8328fa5abbb" }, // 178
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_01.msa",		"40430673d7205417d9952451daf11da0" }, // 179
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_02.msa",		"41292c01fedd999cf2f9f45f548c5e5a" }, // 180
	{ "d:\\ymir work\\pc\\assassin\\horse_onehand_sword\\combo_03.msa",		"cdf20c9fe6d1b0582eef419afaed5077" }, // 181
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_01.msa",		"e6fb9bdcd20616f7000114c6661caef2" }, // 182
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_02.msa",		"bb2763e11c3a16632608f6b04a0a713e" }, // 183
	{ "d:\\ymir work\\pc\\assassin\\horse_dualhand_sword\\combo_03.msa",		"13c608748b3fce510b11596549b68946" }, // 184
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_01.msa",		"d24236f81e0aeff2f685ae9bf1fa6362" }, // 185
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_02.msa",		"c651d1653e87edaacdd149b73d709c20" }, // 186
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_03.msa",		"371b7ae31b5be9d64812b7a0a95150c5" }, // 187
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_04.msa",		"ae2ef8fe64e02da7f988bcb737062dc1" }, // 188
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_05.msa",		"52831f4375e6be175020a7fecd88c575" }, // 189
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_06.msa",		"ca931d7d1fb88a24bd29482ffb26a8f3" }, // 190
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\combo_07.msa",		"6b792ef10ec48e41d0ddf985709a2c95" }, // 191
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage.msa",		"643460a7e6ae9755a0bca0cbd4228541" }, // 192
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_1.msa",		"f27898c4387fccb32385b926c87f053f" }, // 193
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_2.msa",		"14a70c66c0a6704263a17bf40d91440f" }, // 194
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\damage_3.msa",		"14cae205eead54082b96795e15dec0aa" }, // 195
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\wait.msa",		"45fafacabe1a857f03a6fefe57bcac45" }, // 196
	{ "d:\\ymir work\\pc\\assassin\\onehand_sword\\wait_1.msa",		"b3015cc56e760e029cb43fea1bb234fe" }, // 197
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_01.msa",		"7708aa483eb0c380a11025346201d058" }, // 198
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_02.msa",		"4f9693b505bb92393d812fa4faa7a2f7" }, // 199
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_03.msa",		"fec496f6b3e15c48111dabfaf1289d20" }, // 200
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_04.msa",		"924eb0298433f7a3bd27ae99c209b60a" }, // 201
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_05.msa",		"aba2b8f27156cd5ad364fdfa11ca457a" }, // 202
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_06.msa",		"79532fda61e3dab7a6d0c080ed462e4d" }, // 203
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_07.msa",		"527dcf3ac220d4824b2dc76e86e498b6" }, // 204
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\combo_08.msa",		"2da5ddac3e619b2eceb6ea93644bd109" }, // 205
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage.msa",		"1c9abdf3e779b3e9a7af70813a5bf2ae" }, // 206
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_1.msa",		"c63b5d9b17487edff2e74173367e6543" }, // 207
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_2.msa",		"31454e5d262d49124da0421886dc9278" }, // 208
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\damage_3.msa",		"081e408128052ca0e5b778ccfe7a0243" }, // 209
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\wait.msa",		"51fbbf00bff084b5e2f9dacdaf9e9cd4" }, // 210
	{ "d:\\ymir work\\pc\\assassin\\dualhand_sword\\wait_1.msa",		"4f95af0aba137dc73e1a2adf23d8ec6d" }, // 211
	{ "d:\\ymir work\\pc\\assassin\\bow\\wait.msa",		"79bedce37aba3c547abaf1204a775f95" }, // 212
	{ "d:\\ymir work\\pc\\assassin\\bow\\wait_1.msa",		"0f61c0e1dcf1ea05e4ac969d3252e78e" }, // 213
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait.msa",		"ad7b770b62e11a463b1a023e606d9482" }, // 214
	{ "d:\\ymir work\\pc2\\assassin\\horse\\damage.msa",		"1b1e8e92216e51c6eba79f6ee1252421" }, // 215
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait_1.msa",		"b65628d976796882396be37c2e6260cd" }, // 216
	{ "d:\\ymir work\\pc2\\assassin\\horse\\wait_2.msa",		"7f366debaf2cea3331e0850226f1e22f" }, // 217
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_01.msa",		"dadc6462c1cd239218a963f63f6e6b3a" }, // 218
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_02.msa",		"83cf6eead20ef300af9ce9d68f55b5b9" }, // 219
	{ "d:\\ymir work\\pc2\\assassin\\horse_onehand_sword\\combo_03.msa",		"bbd93f7ca23347f58e26b149f78d215c" }, // 220
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_01.msa",		"ec3f3692d5dab712649129df2aa075b8" }, // 221
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_02.msa",		"d4861235b901a7f50d27f452dbc68588" }, // 222
	{ "d:\\ymir work\\pc2\\assassin\\horse_dualhand_sword\\combo_03.msa",		"7efa998c45c487f7767d68945ee6d017" }, // 223
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_01.msa",		"a3f032fa9f2faa0f3d7ed19a441bcf9d" }, // 224
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_02.msa",		"d1bb7ff76603068f7c76463452b2bde8" }, // 225
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_03.msa",		"81b5c276d9d1ab617888a164bde01836" }, // 226
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_04.msa",		"873943e90e5139dc850dfae24f9e6af0" }, // 227
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_05.msa",		"6fb1f5381951fad0238ca749cbb34442" }, // 228
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_06.msa",		"5d1c3b4ee30f70bc9ae46ca45c6ceaad" }, // 229
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\combo_07.msa",		"302ea8fad102ebd7376ffbb448a26023" }, // 230
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage.msa",		"87902cd60ee97743082a6154246e5966" }, // 231
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_1.msa",		"88ca6e5af55d8af73dc9a49a07ba6d7d" }, // 232
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_2.msa",		"e59f9c6a0f784bba7dec8ff57a56c3b7" }, // 233
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\damage_3.msa",		"f12826d61b88224481ca94b837046656" }, // 234
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\wait.msa",		"c54f6bf69694e039234a206b5f64b29d" }, // 235
	{ "d:\\ymir work\\pc2\\assassin\\onehand_sword\\wait_1.msa",		"bf855de2d8aa0750d57903ac0432e908" }, // 236
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_01.msa",		"487d02bfa4499062c94355c0c1fde610" }, // 237
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_02.msa",		"317d3558ea68415aece4d23be6577725" }, // 238
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_03.msa",		"12a01be86e6a8f4330c08bb33c5b11e1" }, // 239
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_04.msa",		"d56e2e0f5c38d4c1c080dbadca989052" }, // 240
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_05.msa",		"f64a43dbc4eda4468d6513c77881e98f" }, // 241
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_06.msa",		"6981a954ab687374965e3f5cc29774f4" }, // 242
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_07.msa",		"c49deb4a7f685e938a564031ffc27e6c" }, // 243
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\combo_08.msa",		"c49deb4a7f685e938a564031ffc27e6c" }, // 244
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage.msa",		"fd7d292dd830c5afda12c0745711d458" }, // 245
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_1.msa",		"8ef1f318f9a8b57e83274ea7c354d30c" }, // 246
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_2.msa",		"9a22acf954cd8f72f69239c7e6697f1a" }, // 247
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\damage_3.msa",		"5277d4e913f93a71071e58f393c2a06d" }, // 248
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\wait.msa",		"5368e10d8cf302d0a2886de0d59de762" }, // 249
	{ "d:\\ymir work\\pc2\\assassin\\dualhand_sword\\wait_1.msa",		"f4b56182da6fe7da06c7fc649505e0b5" }, // 250
	{ "d:\\ymir work\\pc2\\assassin\\bow\\wait.msa",		"4a83cc37ed104876cb032319816b04c1" }, // 251
	{ "d:\\ymir work\\pc2\\assassin\\bow\\wait_1.msa",		"05b9f0658227239349eb803f40f35442" }, // 252
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait.msa",		"876a671bb1bf58a56537872637c8c81b" }, // 253
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\front_damage.msa",		"e84d3b76afb2acb803eb3b2dd113f5e4" }, // 254
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait1.msa",		"3a9c8d00ee09d32367d76238f9404891" }, // 255
	{ "d:\\ymir work\\pc3\\wolfman\\horse\\wait2.msa",		"cdd1251745f274de26bd4025d0a7cb8f" }, // 256
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_01.msa",		"2e05606ae605b0020efb48c78567a5c0" }, // 257
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_02.msa",		"f73ddfd65da257206b56f58bb2ab2ace" }, // 258
	{ "d:\\ymir work\\pc3\\wolfman\\horse_claw\\combo_03.msa",		"ccc86e18cde87559157c6caae0082763" }, // 259
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_01.msa",		"791ca77c6aa6579328ae4186416dad66" }, // 260
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_02.msa",		"a086f6d6dbc823d0feb656223dd4187e" }, // 261
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_03.msa",		"57b7eb48974b3084f8044c22b22b3550" }, // 262
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_04.msa",		"0dd3a47211ea745fa1280290755c1850" }, // 263
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_05.msa",		"78946d88407ed4e0e43f2450739e9e23" }, // 264
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_06.msa",		"3819f1702b52b4b923c88c464359db55" }, // 265
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\combo_07.msa",		"41c5b74fb872b27a600c534763ebac31" }, // 266
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\back_damage.msa",		"4a8deecf0c4c6e0dabe2ed1ac3cd3dcc" }, // 267
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\back_damage1.msa",		"152fd1c8b60fd6f6186fbdfdf727f509" }, // 268
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\front_damage.msa",		"8be1279e45125f52bc9b9eca03b74ecc" }, // 269
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\front_damage1.msa",		"2fb491222616a91ab5380104ea8cd4f0" }, // 270
	{ "d:\\ymir work\\pc3\\wolfman\\claw\\wait.msa",		"30bfce58e5d1f7b1ba6dfcf12a8e70de" }, // 271
	{ "metin2_map_a1\\000000\\attr.atr",		"760a2199a461964e3b3460a9ca8cef99" }, // 274
	{ "metin2_map_a1\\001001\\attr.atr",		"471dbdf339b3f1687f41904527676a64" }, // 275
	{ "metin2_map_a3\\000000\\attr.atr",		"9ef79c0ed2330e3db76df921ae981fd0" }, // 276
	{ "metin2_map_a3\\001001\\attr.atr",		"0d662913588f70e150d0626ae0185525" }, // 277
	{ "metin2_map_b1\\000000\\attr.atr",		"08a3fa27624b39735b4bf409f9a3632a" }, // 278
	{ "metin2_map_b1\\001001\\attr.atr",		"099b99b4adec67fedc53f7615f3aa892" }, // 279
	{ "metin2_map_b3\\000000\\attr.atr",		"ef85978d327801877207a4d8fb73d8c2" }, // 280
	{ "metin2_map_b3\\001001\\attr.atr",		"4d0d9d6a86629131f715ac4598f84827" }, // 281
	{ "metin2_map_c1\\000000\\attr.atr",		"c5a714a3677086aaccf7309edf795782" }, // 282
	{ "metin2_map_c1\\001001\\attr.atr",		"2c2da08f1f02f184ef75c0012b2b53d9" }, // 283
	{ "metin2_map_c3\\000000\\attr.atr",		"856526ae5579c9b598145b0b603e714e" }, // 284
	{ "metin2_map_c3\\001001\\attr.atr",		"07881f0a9c4ec9326dbb3f2c0537979f" }, // 285
	{ "metin2_map_monkeydungeon\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" }, // 286
	{ "metin2_map_monkeydungeon_02\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" }, // 287
	{ "metin2_map_monkeydungeon_03\\001001\\attr.atr",		"208fe753339985c9c260b068bc8b4578" }, // 288
	{ "metin2_map_spiderdungeon\\001001\\attr.atr",		"54671e12e49438f891df56a3ac9e77ea" }, // 289
	{ "metin2_map_trent\\001001\\attr.atr",		"53b7f62e328327776e542a56eeeef8e4" }, // 290
	{ "metin2_map_trent02\\001001\\attr.atr",		"ae1dd100180f1b9d0e7071622062765a" }, // 291

	{ "XXX", 0 }, // DO NOT REMOVE IT
};

#ifdef _DEBUG
void HashDump(const char* cArgFormat, ...) 
{
	char szTmpString[2000];

	va_list vaArgList;
	va_start(vaArgList, cArgFormat);
	wvsprintfA(szTmpString, cArgFormat, vaArgList);
	va_end(vaArgList);

	std::ofstream f("hashs.txt", std::ofstream::out | std::ofstream::app);
	f << szTmpString << std::endl;
	f.close();
}

void DumpMappedFileHashes()
{
	for (size_t i = 0; strcmp(st_FileHashList[i].c_szFileName, "XXX"); ++i)
	{
		auto strCurrentFileName = st_FileHashList[i].c_szFileName;
		auto strHash = GetMappedFileHash(strCurrentFileName);

		HashDump("\t{ \"%s\",		\"%s\" }, // %d", strCurrentFileName, strHash.c_str(), i);
	}
}
#endif

void CAnticheatManager::CheckMappedFiles()
{
	for (size_t i = 0; strcmp(st_FileHashList[i].c_szFileName, XOR("XXX")); ++i)
	{
		auto c_szCurrentFileName = st_FileHashList[i].c_szFileName;
		auto strCurrentFileHash = std::string(st_FileHashList[i].c_szFileMd5);

		auto strCorrectFileHash = GetMappedFileHash(c_szCurrentFileName);

		if (!strCorrectFileHash.empty() && strCorrectFileHash != strCurrentFileHash)
		{
#ifdef _DEBUG
			DumpMappedFileHashes();
#endif
			CCheatDetectQueueMgr::Instance().AppendDetection(MAPPED_FILE_VIOLATION, i, "");
			ExitByAnticheat(MAPPED_FILE_VIOLATION, i, 0, true);
		}
	}
}
