enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG, 
	TEXT_TAG_COLOR, 
	TEXT_TAG_HYPERLINK_START, 
	TEXT_TAG_HYPERLINK_END, 
	TEXT_TAG_RESTORE_COLOR,
};

extern int GetTextTag(const wchar_t * src, int maxLen, int & tagLen, std::wstring & extraInfo);
extern std::wstring GetTextTagOutputString(const wchar_t * src, int src_len);
extern int GetTextTagOutputLen(const wchar_t * src, int len);
extern int FindColorTagEndPosition(const wchar_t * src, int src_len);
extern int FindColorTagStartPosition(const wchar_t * src, int src_len);
extern int GetTextTagInternalPosFromRenderPos(const wchar_t * src, int src_len, int offset);
