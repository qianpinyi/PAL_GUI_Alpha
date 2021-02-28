#ifndef PAL_BASICFUNCTIONS_0
#define PAL_BASICFUNCTIONS_0 1

#include <iostream>
#include <string>
#include <stack>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>
#include <time.h>
#include <cmath>
#include "io.h"

using namespace std;

#define null NULL

void * const CONST_THIS=new int*;
void * const CONST_TRUE=new int*;
void * const CONST_FALSE=new int*;
void * const CONST_Ptr_0=new int*;
void * const CONST_Ptr_1=new int*;
void * const CONST_Ptr_2=new int*;
void * const CONST_Ptr_3=new int*;

/*  Some useful info
-std=c++11
-finput-charset=GBK

-fexec-charset=GBK
-finput-charset=UTF8

-static
-libstdc++ 
-static
-libgcc 
-lmingw32 

-lSDL2main 
-lSDL2 
-lSDL2_image 
-lSDL2_mixer 
-lSDL2_ttf 

-lwsock32 

-llua 

-lavcodec 
-lavdevice 
-lavfilter 
-lavformat  
-lavutil 
-lswscale 
-lswresample 
-lpostproc

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif




*/

/*
template <typename T>  
inline void safe_delete(T *&target) {  
    if (nullptr != target) {  
        delete target;  
        target = nullptr;  
    }  
}  
  
// ɾ������ָ��  
template <typename T>  
inline void safe_delete_arr(T *&target) {  
    if (nullptr != target) {  
        delete[] target;  
        target = nullptr;  
    }  
} 

template <typename T>  
inline void safe_delete_void_ptr(void *&target) {  
    if (nullptr != target) {  
        T* temp = static_cast<T*>(target);  
        delete temp;  
        temp = nullptr;  
        target = nullptr;  
    }  
}  
*/

namespace CharSet_Net
{
	//CharSet.h
//	#pragma once
	#include <iostream>
	#include <string>
	std::string  UnicodeToAnsi(const std::wstring& unicode);
	std::wstring AnsiToUnicode(const std::string& ansi);
	std::string  AnsiToUtf8(const std::string& strSrc);
	std::string  Utf8ToAnsi(const std::string& strSrc);
	std::string  UnicodeToUtf8(const std::wstring& wstrSrc);
	std::wstring Utf8ToUnicode(const std::string& strSrc);
	std::string  GBKToUtf8(const std::string& gbk);
	std::string  Utf8ToGBK(const std::string& utf8);
	std::wstring GB2312ToUnicode(const std::string& gb2312);
	std::string  UnicodeToGB2312(const std::wstring& unicode);
	std::wstring BIG5ToUnicode(const std::string& big5);
	std::string  UnicodeToBIG5(const std::wstring& unicode);
	std::string  FBIG5ToGB2312(const std::string& big5);
	std::string  GB2312ToFBIG5(const std::string gb2312);
	bool IsUTF8(const void* pBuffer, long size);
	
	//CharSet.cpp
//	#inchude "charset.h"
	#include <Windows.h>
	std::string UnicodeToAnsi(const std::wstring& unicode)
	{
	    LPCWCH ptr = unicode.c_str();
	    /** 分配目标空间, 一个16位Unicode字符最多可以转为4个字节int size = static_cast<int>( wstrSrc.size() * 4 + 10 );*/
	    int size = WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, -1, NULL, 0, NULL, NULL);
	std::string strRet(size, 0);
	    int len = WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, -1, (LPSTR)strRet.c_str(), size, NULL, NULL);
	return strRet;
	}
	std::wstring AnsiToUnicode(const std::string& ansi)
	{
	    LPCCH ptr = ansi.c_str();
	    int size = MultiByteToWideChar(CP_ACP, 0, ptr, -1, NULL, NULL
		);
	std::wstring wstrRet(size, 0);
	    int len = MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)wstrRet.c_str(), size);
	return wstrRet;
	}
	std::string AnsiToUtf8(const std::string& ansi)
	{
	    LPCCH ptr = ansi.c_str();
	    /* 分配目标空间, 长度为 Ansi 编码的两倍 */
	    int size = MultiByteToWideChar(CP_ACP, 0, ptr, -1, NULL, NULL);
	std::wstring wstrTemp(size, 0);
	    int len = MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)wstrTemp.c_str(), size);
	return UnicodeToUtf8(wstrTemp);
	}
	std::string Utf8ToAnsi(const std::string& utf8)
	{
	    std::wstring wstrTemp = Utf8ToUnicode(utf8);
	LPCWCH ptr = wstrTemp.c_str();
	    int size = WideCharToMultiByte(CP_ACP, 0, ptr, -1, NULL, 0, NULL, NULL);
	std::string strRet(size, 0);
	    int len = WideCharToMultiByte(CP_ACP, 0, ptr, -1, (LPSTR)strRet.c_str(), size, NULL, NULL);
	return strRet;
	}
	std::string UnicodeToUtf8(const std::wstring& unicode)
	{
	    /* 分配目标空间, 一个16位Unicode字符最多可以转为4个字节 */
	    LPCWCH ptr = unicode.c_str();
	    int size = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, NULL, 0, NULL, NULL);
	std::string strRet(size, 0);
	    int len = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, (char*)strRet.c_str(), size, NULL, NULL);
	return strRet;
	}
	std::wstring Utf8ToUnicode(const std::string& utf8)
	{
	    LPCCH ptr = utf8.c_str();
	    int size = MultiByteToWideChar(CP_UTF8, 0, ptr, -1, NULL, NULL);
	std::wstring wstrRet(size, 0);
	    int len = MultiByteToWideChar(CP_UTF8, 0, ptr, -1, (LPWSTR)wstrRet.c_str(), size);
	return wstrRet;
	}
	std::string GBKToUtf8(const std::string& gbk)
	{
	    return AnsiToUtf8(gbk);
	}
	std::string Utf8ToGBK(const std::string& utf8)
	{
	    return Utf8ToAnsi(utf8);
	}
	bool IsUTF8(const void* pBuffer, long size)
	{
	    bool isUTF8 = true;
	    unsigned char* start = (unsigned char*)pBuffer;
	    unsigned char* end = (unsigned char*)pBuffer + size;
	    while (start < end)
	    {
	        if (*start < 0x80) { /*(10000000): 值小于0x80的为ASCII字符*/
	            start++;
	        }
	        else if (*start < (0xC0)) { /*(11000000): 值介于0x80与0xC0之间的为无效UTF-8字符*/
	            isUTF8 = false;
	            break;
	        }
	        else if (*start < (0xE0)) { /*(11100000): 此范围内为2字节UTF-8字符  */
	            if (start >= end - 1) {
	                break;
	            }
	            if ((start[1] & (0xC0)) != 0x80) {
	                isUTF8 = false;
	                break;
	            }
	            start += 2;
	        }
	        else if (*start < (0xF0)) { /**(11110000): 此范围内为3字节UTF-8字符*/
	            if (start >= end - 2) {
	                break;
	            }
	            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80) {
	                isUTF8 = false;
	                break;
	            }
	            start += 3;
	        }
	        else {
	            isUTF8 = false;
	            break;
	        }
	    }
	return isUTF8;
	}
	//GB2312 转换成 Unicode
	std::wstring GB2312ToUnicode(const std::string& gb2312)
	{
	    UINT nCodePage = 936; //GB2312
	    int size = MultiByteToWideChar(nCodePage, 0, gb2312.c_str(), -1, NULL, 0);
	std::wstring wstrRet(size, 0);
	    MultiByteToWideChar(nCodePage, 0, gb2312.c_str(), -1, (LPWSTR)wstrRet.c_str(), size);
	return wstrRet;
	}
	//BIG5 转换成 Unicode
	std::wstring BIG5ToUnicode(const std::string& big5)
	{
	    UINT nCodePage = 950; //BIG5
	    int size = MultiByteToWideChar(nCodePage, 0, big5.c_str(), -1, NULL, 0);
	std::wstring wstrRet(size, 0);
	    MultiByteToWideChar(nCodePage, 0, big5.c_str(), -1, (LPWSTR)wstrRet.c_str(), size);
	return wstrRet;
	}
	//Unicode 转换成 GB2312
	std::string UnicodeToGB2312(const std::wstring& unicode)
	{
	    UINT nCodePage = 936; //GB2312
	    int size = WideCharToMultiByte(nCodePage, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
	std::string strRet(size, 0);
	    WideCharToMultiByte(nCodePage, 0, unicode.c_str(), -1, (LPSTR)strRet.c_str(), size, NULL, NULL);
	return strRet;
	}
	//Unicode 转换成 BIG5
	std::string UnicodeToBIG5(const std::wstring& unicode)
	{
	    UINT nCodePage = 950; //BIG5
	    int size = WideCharToMultiByte(nCodePage, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
	std::string strRet(size, 0);
	    WideCharToMultiByte(nCodePage, 0, unicode.c_str(), -1, (LPSTR)strRet.c_str(), size, NULL, NULL);
	return strRet;
	}
	//繁体中文BIG5 转换成 简体中文 GB2312
	std::string FBIG5ToGB2312(const std::string& big5)
	{
	    LCID lcid = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRC);
	    std::wstring unicode = BIG5ToUnicode(big5);
	std::string gb2312 = UnicodeToGB2312(unicode);
	    int size = LCMapStringA(lcid, LCMAP_SIMPLIFIED_CHINESE, gb2312.c_str(), -1, NULL, 0);
	std::string strRet(size, 0);
	    LCMapStringA(0x0804, LCMAP_SIMPLIFIED_CHINESE, gb2312.c_str(), -1, (LPSTR)strRet.c_str(), size);
	return strRet;
	}
	//简体中文 GB2312 转换成 繁体中文BIG5
	std::string GB2312ToFBIG5(const std::string gb2312)
	{
	    LCID lcid = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRC);
	    int size = LCMapStringA(lcid, LCMAP_TRADITIONAL_CHINESE, gb2312.c_str(), -1, NULL, 0);
	std::string strRet(size, 0);
	    LCMapStringA(lcid, LCMAP_TRADITIONAL_CHINESE, gb2312.c_str(), -1, (LPSTR)strRet.c_str(), size);
	std::wstring unicode = GB2312ToUnicode(strRet);
	    std::string big5 = UnicodeToBIG5(unicode);
	return big5;
	}
}


//int DDflag=1;
enum DebugOut_Channel
{
	DebugOut_Off=0,
	DebugOut_CERR,
	DebugOut_CERR_LOG,
};

ofstream DebugOut_fout("DebugOutLog.txt");

class Debug_Out
{
	public:
		#define DefaultDebugOut_Flag DebugOut_CERR
		int DebugOn=1;
		DebugOut_Channel CurrentOutMode=DefaultDebugOut_Flag;
		
		const Debug_Out& operator % (DebugOut_Channel X)
		{
			CurrentOutMode=X;
			return *this;
		}
		
		template <typename T> const Debug_Out& operator << (T X) const
		{
			if (DebugOn)
				switch (CurrentOutMode)
				{
					case DebugOut_Off: break;
					case DebugOut_CERR: cerr<<X; break;
					case DebugOut_CERR_LOG: cerr<<X; DebugOut_fout<<X; break;
				}
			return *this;
		}
}DD;

class Test_ConDeStructorClass
{
	public:
		int ID;
		
		Test_ConDeStructorClass()
		{
			static int _ID=0;
			ID=++_ID;
			DD<<"Test_ConDeStructorClass Con "<<ID<<"\n";
		}
		
		~Test_ConDeStructorClass()
		{
			DD<<"Test_ConDeStructorClass De "<<ID<<"\n";
		}
};

void Debug_PrintStr(string str)
{
	DD<<"Debug_PrintStr: "<<str<<"\n"; 
	for (int i=0;i<str.length();++i)
		DD<<str[i]<<"|";
	DD<<"\n";
	for (int i=0;i<str.length();++i)
		DD<<(int)str[i]<<"|";
	DD<<"\n";
}

int strTOint(string str)
{
	int x=0;
	for (int i=0;i<str.length();++i)
		x*=10,x+=str[i]-'0';
	return x;
}

long long strTOll(string str)
{
	long long x=0;
	for (int i=0;i<str.length();++i)
		x*=10,x+=str[i]-'0';
	return x;
}

double strTOdb(string str)
{
	double x=0;
	for (int i=0,flag=0;i<str.length();++i)
		if (str[i]=='.') flag=1;
		else if (flag==0) x*=10,x+=str[i]-'0';
		else x+=(double(str[i]-'0'))/(flag*=10);
	return x;
}

string llTOstr(long long x)
{
	if (x==0) return "0"; 
	string re;
	if (x<0) re+="-",x=-x;
	stack <char> sta;
	while (x)
		sta.push(x%10+'0'),x/=10;
	while (!sta.empty())
		re+=sta.top(),sta.pop();
	return re;
}

char *GetTime1()
{
	time_t rawtime;
	time(&rawtime);
	return ctime(&rawtime);
}

string ReplaceCharInStr(string str,int L,int R,char ch1,char ch2)
{
	for (int i=L;i<=R;++i)
		if (str[i]==ch1)
			str[i]=ch2;
	return str;
}

string ReplaceCharInStr(string str,char ch1,char ch2)
{
	return ReplaceCharInStr(str,0,str.length()-1,ch1,ch2);
}

string DeletePreBlank(string str)
{
	for (int i=0;i<str.length();++i)
		switch (str[i])
		{
			case '\n':break;//??
			case '\r':break;//??
			case '\t':break;
			case ' ' :break;
			case '\0':break;
			default:
				return str.substr(i,str.length()-i);
		}
	return "";
}

string DeleteEndBlank(string str)
{
	for (int i=str.length()-1;i>=0;--i)
		switch (str[i])
		{
			case '\n':break;//??
			case '\r':break;//??
			case '\t':break;
			case ' ' :break;
			case '\0':break;
			default:
				return str.substr(0,i+1);
		}
	return "";
}

wstring DeleteEndBlank(wstring wstr)
{
	for (int i=wstr.length()-1;i>=0;--i)
		switch (wstr[i])
		{
			case L'\n':break;//??
			case L'\r':break;//??
			case L'\t':break;
			case L' ' :break;
			case L'\0':break;
			default:
				return wstr.substr(0,i+1);
		}
	return L"";
}

string CutFirstCharInvolveSubStr(string str,char ch)
{
	int pos1=0,pos2=0;
	pos1=str.find(ch);
	if (pos1<str.length()-1)
		pos2=str.find(pos1+1,ch);
	if (pos1<pos2) return str.substr(pos1+1,pos2-pos1-1);
	else return "";
}

string GetAftername(string str)
{
	int p=str.rfind(".",str.length()-1);
	if (p==-1) return "";
	return str.substr(p,1e9);
}

string GetWithOutAftername(string str)
{
	int p=str.rfind(".",str.length()-1);
	if (p==-1) return str;
	return str.substr(0,p);
}

string GetLastBeforeBackSlash(string str)
{
	return str.substr(str.rfind("\\",str.length()-1)+1,1e9);
}

string GetPreviousBeforeBackSlash(string str)
{
	int p=str.rfind("\\",str.length()-1);
	if (p==-1) return "";
	return str.substr(0,p);
}

string Atoa(string str)
{
	for (int i=0;i<str.length();++i)
		if ('A'<=str[i]&&str[i]<='Z')
			str[i]+='a'-'A';
	return str;
}

string atoA(string str)
{
	for (int i=0;i<str.length();++i)	
		if ('a'<=str[i]&&str[i]<='z')
			str[i]+='A'-'a';
	return str;
}

void ReplaceXMLescapecharWithReal(string &str)
{
	while (1)
	{
		int c=str.find("&amp;");
		if (c==-1) break;
		str.replace(c,5,"&");
	}
	while (1)
	{
		int c=str.find("&lt;");
		if (c==-1) break;
		str.replace(c,4,"<");
	}
	while (1)
	{
		int c=str.find("&gt;");
		if (c==-1) break;
		str.replace(c,4,">");
	}
	while (1)
	{
		int c=str.find("&apos;");
		if (c==-1) break;
		str.replace(c,6,"'");
	}
	while (1)
	{
		int c=str.find("&quot;");
		if (c==-1) break;
		str.replace(c,6,"\"");
	}
}

void GetRidOfEndChar0(string &str)
{
	while (!str.empty()&&(*str.rbegin())==0)
		str.erase(str.end()-1);
}

void GetRidOfEndChar0(wstring &wstr)
{
	while (!wstr.empty()&&(*wstr.rbegin())==0)
		wstr.erase(wstr.end()-1);
}

class PathWithType
{
	public:
		int type=0,usercode=0;//user define
		string path,appearance;
		void *userdata=NULL;
		
		void Set(int _type,string _path)
		{type=_type;path=_path;}
		
		void Set(int _type,int _usercode,string _path,string _appear)
		{type=_type;usercode=_usercode;path=_path;appearance=_appear;}
		
		void Set(int _type,int _usercode,string _path,string _appear,void *_userdata)
		{type=_type;usercode=_usercode;path=_path;appearance=_appear;userdata=_userdata;}
		
		PathWithType(){}
		
		PathWithType(int _type,string _path)
		{type=_type;path=_path;}
		
		PathWithType(int _type,int _usercode,string _path,string _appear)
		{type=_type;usercode=_usercode;path=_path;appearance=_appear;}
		
		PathWithType(int _type,int _usercode,string _path,string _appear,void *_userdata)
		{type=_type;usercode=_usercode;path=_path;appearance=_appear;userdata=_userdata;}
};

void systemUTF8(string str)
{
	DD<<"systemUTF8("<<str<<")\n";
	_wsystem(CharSet_Net::Utf8ToUnicode(str).c_str());
}

void SelectInWinExplorer/*_Start*/(string path)
{
	systemUTF8("explorer /select, \""+path+"\"");
}

namespace ShellFileOperation
{
	vector <string> GetAllFile(string path,bool fileFlag)//fileFlag: 0:Directory 1:File
	{
		path="\""+path+"\"";
		string str="dir "+path+(fileFlag?" /a-d":" /ad")+" /b >> GetAllFileTemp.txt";
		systemUTF8(str);
		ifstream fin("GetAllFileTemp.txt");
		vector <string> ret;
		while (getline(fin,str)) ret.push_back(str);
		fin.close();
		system("del GetAllFileTemp.txt");
		return ret;
	}
	
	void SFO_Init()
	{
		system("chcp 65001");
	}
}
#define SFO ShellFileOperation 

vector <string> GetAllFile_UTF8(string str,bool fileFlag)
{
	vector <string> ret;
	wstring wstr=CharSet_Net::Utf8ToUnicode(str+"\\*");
	int hFiles=0;
	_wfinddata_t da;
	if ((hFiles=_wfindfirst(wstr.c_str(),&da))!=-1)
	{
		do
		{
			str=DeleteEndBlank(CharSet_Net::UnicodeToUtf8(da.name));
			//str.erase(str.length()-1);
			if (da.attrib&_A_SUBDIR)
			{
				if (!fileFlag)
					if (str!="."&&str!="..")
						ret.push_back(str);
			}
			else
			{
				if (fileFlag)
					ret.push_back(str);
			}
		}
		while (_wfindnext(hFiles,&da)==0);
	}
	_findclose(hFiles);
	return ret;
}



class PAL_Config_Alpha
{
	public:
		#define PAL_Config_Alpha_Version "PAL_Config_Alpha_Version_1.0"
		string cfgfile;
		map <string,vector <string> > ma;
		map <string,vector <string> >::iterator mp;
		bool OpenedSuccessfully=0;
		bool Locked=0,NextLocked=0;
		
		enum
		{
			CfgFileNotOpen=1,
			WrongVersion,
			WrongEndFlag,
			UnableSovle,
			
			NotOpenSucessfully,
			CfgLocked
			
		};
		
		string GetValue(int p)
		{
			if (mp!=ma.end())
				if (p>=0&&p<mp->second.size())
					return mp->second[p];
			return "";
		}
		
		string GetValue(const string key,int p)
		{
			mp=ma.find(key);
			return GetValue(p);
		}
		
		vector <string> * GetAllValue()
		{
			if (mp!=ma.end())
				return &mp->second;
			return NULL;
		}
		
		vector <string> * GetAllValue(const string key)
		{
			mp=ma.find(key);
			return GetAllValue();
		}
		
		string GetValue()
		{return GetValue(0);}
		
		string GetValue(const string key)
		{return GetValue(key,0);}
		
		int GetValueCnt()
		{
			if (mp!=ma.end())
				return mp->second.size();
			return 0;
		}
		
		int GetValueCnt(const string key)
		{
			mp=ma.find(key);
			return GetValueCnt();
		}
		
		void SetValue(const string key,string value,int p)
		{
			mp=ma.find(key);
			if (mp!=ma.end())
				if (p>=0&&p<mp->second.size())
					mp->second[p]=value;
				else if (p==mp->second.size())
					mp->second.push_back(value);
		}
		
		void AddValue(string value)
		{
			if (mp!=ma.end())
				mp->second.push_back(value);
		}
		
		void AddValue(const string key,string value)
		{
			mp=ma.find(key);
			AddValue(value);
		}
		
		void DeleteValueBack()
		{
			if (mp!=ma.end())
				if (mp->second.size()>0)
					mp->second.erase(mp->second.end());
		}
		
		void DeleteValueBack(const string key)
		{
			mp=ma.find(key);
			DeleteValueBack();
		}
		
		void ClearValue(const string key)
		{
			mp=ma.find(key);
			if (mp!=ma.end())
				ma.erase(mp);
		}
		
		void SetValue(const string key,string value)
		{
			ClearValue(key);
			AddValue(value);
		}
		
		void SetNextLocked(bool _lo)
		{NextLocked=_lo;}
		
		int Read()
		{
			OpenedSuccessfully=0;
			ma.clear();
			ifstream fin(cfgfile.c_str());
			if (fin.is_open())
			{
				string Time_Check_Str,Cfg_Version,tmp;
				fin>>Time_Check_Str
				   >>tmp>>Cfg_Version
				   >>tmp>>Locked
				   ;
				while (Time_Check_Str[0]<0) Time_Check_Str.erase(0,1);
				if (Cfg_Version==PAL_Config_Alpha_Version)
				{
					string stra,strb,strc;
					while (fin>>stra)
					{
						strb.clear();strc.clear();
						getline(fin,strb);
						int stat=0;//0:solve Blank/table char  1:solve common value  2:solve WithBlankValue
						for (int i=0;i<strb.length();++i)
							if (stat==0)
								if (strb[i]==' '||strb[i]=='\t') continue;
								else if (strb[i]=='{') stat=2,strc="";
								else stat=1,strc=strb[i];
							else if (stat==1)
								if (strb[i]==' '||strb[i]=='\t') stat=0,ma[stra].push_back(strc);
								else strc+=strb[i];
							else if (stat==2)
								if (strb[i]=='}'&&(i==strb.length()-1||strb[i+1]==' '||strb[i+1]=='\t')) stat=0,ma[stra].push_back(strc);
								else strc+=strb[i];
						if (stat!=0&&!strc.empty()) ma[stra].push_back(strc);
					}
					fin.close();
//					DD<<"sssss "<<stra<<" "<<Time_Check_Str<<"\n";
//					Debug_PrintStr(stra);
//					Debug_PrintStr(Time_Check_Str);
					if (stra==Time_Check_Str)
					{
						ma.erase(stra);
						return OpenedSuccessfully=1,0;
					}
					
					return WrongEndFlag;
				}
				fin.close();
				return WrongVersion;
			}
			else return CfgFileNotOpen;
		}
		
		int Write()
		{
			if (!OpenedSuccessfully) return NotOpenSucessfully;
			if (Locked) return CfgLocked;
			ofstream fout(cfgfile.c_str());
			if (fout.is_open())
			{
				string OutPut_Time_Flag=GetTime1();
				fout<<ReplaceCharInStr(OutPut_Time_Flag,' ','_')
					<<"Version "<<PAL_Config_Alpha_Version<<endl
					<<"Locked "<<NextLocked<<endl
					<<endl;
				for (map <string,vector <string> >::iterator p=ma.begin();p!=ma.end();++p)
				{
					fout<<p->first;
					for (int i=0;i<p->second.size();++i)
						if (p->second[i]=="")
							fout<<" {}";
						else if (p->second[i].find(' ')==-1)
							fout<<" "<<p->second[i];
						else fout<<" {"<<p->second[i]<<"}";
					fout<<endl;
				}
				
				fout<<endl<<ReplaceCharInStr(OutPut_Time_Flag,' ','_');
				fout.close();
				return 0;
			}
			else return CfgFileNotOpen;
		}
		
		PAL_Config_Alpha(string _cfgfile)
		{
			cfgfile=_cfgfile;
		}
};

void FindFiles_R(string str,int dep)//18.10.14
{
	int hFiles=0;
	_finddata_t da;
	if ((hFiles=_findfirst((str+"\\*").c_str(),&da))!=-1)
	{
		do
		{
			//TAB(dep);cout<<"["<<da.name<<"]"<<endl;
			for (int i=1;i<=dep;++i) cout<<"-";
			if (da.attrib&_A_SUBDIR)
				cout<<"#";
			else cout<<" ";
			cout<<"["<<str+"\\"+da.name<<"]"<<endl;
			if (da.attrib&_A_SUBDIR)
				if (strcmp(da.name,"..")!=0&&strcmp(da.name,".")!=0)
					FindFiles_R(str+"\\"+da.name,dep+1);
		}
		while (_findnext(hFiles,&da)==0);
	}
	_findclose(hFiles);
}

void FindFiles(string str,int dep)//18.10.14
{
	int hFiles=0;
	_finddata_t da;
	if ((hFiles=_findfirst((str+"\\*").c_str(),&da))!=-1)
	{
		do
		{
			//TAB(dep);cout<<"["<<da.name<<"]"<<endl;
			if (da.attrib&_A_SUBDIR)
				cout<<"#";
			else cout<<" ";
			cout<<"["<<str+"\\"+da.name<<"]"<<endl;
//			if (da.attrib&_A_SUBDIR)
//				if (strcmp(da.name,"..")!=0&&strcmp(da.name,".")!=0)
//					FindFiles(str+"\\"+da.name,dep+1);
		}
		while (_findnext(hFiles,&da)==0);
	}
	_findclose(hFiles);
}

//int EXSTYLE = GetWindowLong(rhwnd, GWL_EXSTYLE); //获取窗口风格
//SetWindowLong(rhwnd, GWL_EXSTYLE, EXSTYLE | WS_EX_LAYERED);  //设置透明窗口风格


#endif
