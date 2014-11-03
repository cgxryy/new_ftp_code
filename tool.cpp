// =====================================================================================
// 
//       Filename:  tool.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2014年11月02日 14时14分02秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Reazon (Changgongxiaorong), cgxryy@gmail.com
//        Company:  Class 1203 of Network Engineering
// 
// =====================================================================================
#include "tool.h"
#include <string.h>
#include <iostream>
using namespace::std;

char* tool_str::path_change(char* old_path)
{
	//do like /home/chang -> ./home/chang
	if (strlen(old_path) >= 256)
	      return NULL;
	
	string old_str = old_path;
	string del_str = "../";
	string del_str1 = "//";
	string rep_str = "/";
	replace_all(old_str, del_str, rep_str);
	replace_all(old_str, del_str1, rep_str);
	
	char path[256] = ".";
	strncat(path, old_path, strlen(old_path));
	path[strlen(old_path)+1] = '\0';
	strncpy(old_path, path, strlen(path));

	return old_path;
}

void tool_str::replace_all(string& targetstring, string const & substring, string const & replacestring)
{
	string::size_type pos = targetstring.find(substring);
	string::size_type sub_size = substring.size();
	string::size_type replace_size = replacestring.size();
	while (pos != string::npos)
	{
		targetstring.replace(pos, sub_size, replacestring);
		pos = targetstring.find(substring, pos + replace_size);
	}
}

char* tool_str::get_str_path(char* str)
{
	//find some ch last appear
	char* ptr;
	ptr = strrchr(str, '/');
	if (ptr)
	{	
		*ptr = '\0';
		return str;
	}
	else 
	      return NULL;
}

