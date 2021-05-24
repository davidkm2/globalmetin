#pragma once
#include <string>
using namespace std;

class CFilename
{
	public:
		CFilename() { }
		CFilename(const char* pFilename) { m_sRaw = pFilename; }
		CFilename(std::string strFilename) { m_sRaw = strFilename; }

		virtual ~CFilename() {}

		operator const string() const { return m_sRaw; }
		operator string&() { return m_sRaw; }
		CFilename& operator =(const CFilename& r) { m_sRaw = r.m_sRaw; return *this; }
		bool operator ==(const CFilename& r) const { return m_sRaw == r.m_sRaw; }
		CFilename operator +(const CFilename& r) const { return CFilename(m_sRaw + r.m_sRaw); }
		CFilename& operator +=(const CFilename& r) { m_sRaw += r.m_sRaw; return *this; }
		const char& operator[](size_t nIdx) const { return m_sRaw[nIdx]; }
		const char* c_str() const { return m_sRaw.c_str(); }
		size_t find(const char* pcszSrc) const { return m_sRaw.find(pcszSrc); }
		bool empty() const { return m_sRaw.empty(); }
		size_t size() const { return m_sRaw.size(); }
		size_t length() const { return m_sRaw.length(); }

		string& GetString() { return m_sRaw; }

		void ChangeDosPath()
		{
			size_t nLength = m_sRaw.length();

			for (size_t i = 0; i < nLength; ++i)
			{
				if (m_sRaw.at(i) == '/')
					m_sRaw.at(i) = '\\';
			}
		}

		void StringPath()
		{
			size_t nLength = m_sRaw.length();

			for (size_t i = 0; i<nLength; ++i)
			{
				if (m_sRaw.at(i) == '\\')
					m_sRaw.at(i) = '/';
				else
					m_sRaw.at(i) = (char)tolower(m_sRaw.at(i));
			}
		}

		string GetName(void);           
		string GetExtension(void);      
		string GetPath(void);           
		string NoExtension(void);       
		string NoPath(void);            

		int compare (const char* s) const { return m_sRaw.compare(s); }
		friend CFilename operator +(const string alfa, const CFilename& beta);

		string m_sRaw;
};

inline CFilename operator +(const string alfa, const CFilename& beta) { return beta + alfa; }

inline string CFilename::GetName(void)
{
	string strName;

	size_t nLength = m_sRaw.length();

	if (nLength > 0)
	{
		size_t iExtensionStartPos = nLength - 1;

		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (m_sRaw[i] == '.')
			{
				iExtensionStartPos = i;
			}

			if (m_sRaw[i] == '/')
			{
				strName = string(m_sRaw.c_str() + i + 1);
				strName.resize(iExtensionStartPos - i - 1);
				break;
			}
		}
	}

	return strName;
}
inline string CFilename::GetExtension(void)
{
	string strExtension;

	size_t nLength = m_sRaw.length();

	if (nLength > 0)
	{
		for (size_t i = nLength - 1; i > 0 && m_sRaw[i] != '/'; i--)
			if (m_sRaw[i] == '.')
			{
				strExtension = string(m_sRaw.c_str( ) + i + 1);
				break;
			}
	}

	return strExtension;
}
inline string CFilename::GetPath(void)
{
	char szPath[1024];
	szPath[0] = '\0';

	size_t nLength = m_sRaw.length();

	if (nLength > 0)
	{
		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (m_sRaw[i] == '/' || m_sRaw[i] == '\\')
			{
				for (size_t j = 0; j < i + 1; j++)
					szPath[j] = m_sRaw[j];
				szPath[i+1] = '\0';
				break;
			}

			if (0 == i)
				break;
		}
	}
	return szPath;
}
inline string CFilename::NoExtension(void)
{
	std::size_t npos = m_sRaw.find_last_of('.');

	if (string::npos != npos)
		return std::string(m_sRaw, 0, npos);

	return m_sRaw;
}
inline string CFilename::NoPath(void)
{
	char szPath[1024];
	szPath[0] = '\0';

	size_t nLength = m_sRaw.length();

	if (nLength > 0)
	{
		strcpy(szPath, m_sRaw.c_str());

		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (m_sRaw[i] == '/' || m_sRaw[i] == '\\')
			{
				int k = 0;
				for (size_t j = i + 1; j < nLength; j++, k++)
					szPath[k] = m_sRaw[j];
				szPath[k] = '\0';
				break;
			}

			if (0 == i)
				break;
		}
	}

	return szPath;
}

class CFileNameHelper
{
public:
	static void ChangeDosPath(string& str) {
		size_t nLength = str.length();

		for (size_t i = 0; i < nLength; ++i)
		{
			if (str.at(i) == '/')
				str.at(i) = '\\';
		}
	}

	static void StringPath(string& str) {
		size_t nLength = str.length();

		for (size_t i = 0; i<nLength; ++i)
		{
			if (str.at(i) == '\\')
				str.at(i) = '/';
			else
				str.at(i) = (char)tolower(str.at(i));
		}
	}

	static string GetName(string& str);           
	static string GetExtension(string& str);      
	static string GetPath(string& str);           
	static string NoExtension(string& str);       
	static string NoPath(string& str);            
};

inline string CFileNameHelper::GetName(string& str)
{
	string strName;

	size_t nLength = str.length();

	if (nLength > 0)
	{
		size_t iExtensionStartPos = nLength - 1;

		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '.')
			{
				iExtensionStartPos = i;
			}

			if (str[i] == '/')
			{
				strName = string(str.c_str() + i + 1);
				strName.resize(iExtensionStartPos - i - 1);
				break;
			}
		}
	}

	return strName;
}

inline string CFileNameHelper::GetExtension(string& str)
{
	string strExtension;

	size_t nLength = str.length();

	if (nLength > 0)
	{
		for (size_t i = nLength - 1; i > 0 && str[i] != '/'; i--)
			if (str[i] == '.')
			{
				strExtension = string(str.c_str( ) + i + 1);
				break;
			}
	}

	return strExtension;
}

inline string CFileNameHelper::GetPath(string& str)
{
	char szPath[1024];
	szPath[0] = '\0';

	size_t nLength = str.length();

	if (nLength > 0)
	{
		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '/' || str[i] == '\\')
			{
				for (size_t j = 0; j < i + 1; j++)
					szPath[j] = str[j];
				szPath[i+1] = '\0';
				break;
			}

			if (0 == i)
				break;
		}
	}
	return szPath;
}

inline string CFileNameHelper::NoExtension(string& str)
{
	std::size_t npos = str.find_last_of('.');

	if (string::npos != npos)
		return std::string(str, 0, npos);

	return str;
}

inline string CFileNameHelper::NoPath(string& str)
{
	char szPath[1024];
	szPath[0] = '\0';

	size_t nLength = str.length();

	if (nLength > 0)
	{
		strcpy(szPath, str.c_str());

		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '/' || str[i] == '\\')
			{
				int k = 0;
				for (size_t j = i + 1; j < nLength; j++, k++)
					szPath[k] = str[j];
				szPath[k] = '\0';
				break;
			}

			if (0 == i)
				break;
		}
	}

	return szPath;
}
