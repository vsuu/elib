#include "strop.h"

__LIB_NAME_SPACE_BEGIN__

unsigned char char2hex_table[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };
//
//int char2hex(char c, unsigned char&r)
//{
//    if (c >= '0' && c <= '9')
//    {
//        r = c - '0';
//    }
//    else
//        if (c >= 'A' && c <= 'F')
//        {
//            r = c - 'A' + 10;
//        }
//        else
//            if (c >= 'a' && c <= 'f')
//            {
//                r = c - 'a' + 10;
//            }
//            else
//            {
//                return 1;
//            }
//    return 0;
//}

char hex2char_table[] = "0123456789ABCDEF";

std::string wstr2str(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size();
	char *_Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	
	size_t count=0;
	size_t pos = 0;
	while (pos<ws.size())
	{
		count+=wcstombs(_Dest+count, _Source+pos, _Dsize-count);
		pos = ws.find(L'\0',pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		else
		{
			count++;
			pos ++;
		}
	};
	
	std::string result(_Dest,count);
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}
std::wstring str2wstr(const std::string &s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	size_t count = 0;
	size_t pos = 0;
	while (pos<s.size())
	{
		count+=mbstowcs(_Dest+count, _Source+pos, _Dsize-count);
		pos = s.find('\0', pos);
		if (pos == std::string::npos)
		{
			break;
		}
		else
		{
			count++;
			pos++;
		}
	}
	std::wstring result(_Dest,count);
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

using std::tuple;
using std::string;
tuple<string, string, string> ParseFileName(const string &filename)
{
	string path, name, suffix;
	path = filename;
	TrimLeft(path, " ");
	TrimRight(path, "\\/ ");
	//parse path;
	auto pos = path.find_last_of("\\/");
	if (pos != string::npos)
	{
		name = path.substr(pos + 1);
		path.erase(pos + 1);
	}
	else
	{
		name = path;
		path.clear();
	}

	pos = name.rfind('.');
	if (pos != string::npos)
	{
		suffix = name.substr(pos + 1);
		name.erase(pos);
	}

	return make_tuple(std::move(path), std::move(name), std::move(suffix));
}


__LIB_NAME_SPACE_END__