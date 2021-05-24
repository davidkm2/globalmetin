#ifndef FOXFS_FILESYSTEM_H
#define FOXFS_FILESYSTEM_H

#include <string>
#include <map>

#include "Archive.h"

#include <cryptopp/md5.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#include <windows.h>      
#include <intrin.h>       
#include <iphlpapi.h>   
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>          
#include <errno.h>           
#include <sys/types.h>       
#include <sys/socket.h>      
#include <sys/ioctl.h>  
#include <sys/resource.h>    
#include <sys/utsname.h>       
#include <netdb.h>           
#include <netinet/in.h>      
#include <netinet/in_systm.h>                 
#include <netinet/ip.h>      
#include <netinet/ip_icmp.h> 
#include <linux/if.h>        
#include <linux/sockios.h>  
#include <pthread.h>
#include <arpa/inet.h>
#endif

namespace FoxFS
{
	
	class FileSystem
	{		
	private:
		struct KeyInfo
		{
			unsigned char key[32];
			unsigned char iv[32];
		};
		
	public:
		FileSystem();
		~FileSystem();
		
		int load(const wchar_t* filename);
		int unload(const wchar_t* filename);
		
		unsigned int size(const char* filename) const;
		int exists(const char* filename) const;
		int get(const char* filename, void* buffer, unsigned int maxsize, unsigned int* outsize) const;
		
	private:
		std::map<std::basic_string<wchar_t>, Archive*> archives;
		
		std::map<std::string, KeyInfo> keys;
	
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
		mutable CRITICAL_SECTION mutex;
#else
		mutable pthread_mutex_t mutex;
#endif
	};
	
}

#endif