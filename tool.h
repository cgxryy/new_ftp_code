// =====================================================================================
// 
//       Filename:  tool.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2014年11月01日 19时47分27秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Reazon (Changgongxiaorong), cgxryy@gmail.com
//        Company:  Class 1203 of Network Engineering
// 
// =====================================================================================
#pragma once

#include <unistd.h>
#include <string.h>
#include <string>

using namespace::std;

class tool_sockfd
{
	public
		:
		char recv_buf[1024];
		//非阻塞的recv  MSG_WAITALL
		static int recv_waitall(int sockfd, void* buf, size_t len, int flags);
};

/**
 * 路径转换
 * 下载时，不能看到指定路径外的文件,任何路径都会变成某文件夹下的文件
 * 上传时，可以选择所有路径,但是不能上传到越权文件夹下
 */
class tool_str
{
	public 
		:
	//绝对路径转化相对路径
	static char* path_change(char* old_path);
	//path_change函数用到的全局替换
	static void replace_all(string& targetstring, string const & substring1, string const & replacestring);
	//dirname 因为系统调用有某些问题
	static char* get_str_path(char* str);
};
