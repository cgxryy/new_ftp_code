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
#include "package.h"
#include <vector>
using namespace::std;

class tool_sockfd
{
	public
		:
		char recv_buf[1024];
	
	static void addfd(int epoll, int fd, bool oneshot);
	static int setnonblocking(int fd);
	static void reset_oneshot(int epollfd, int fd);
};

/**
 * 工具类
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
	static void get_file_name(vector<string> &file_container, buf_data &name_package);
	static char* path_dir(char* old_path);

	static const char* copy_file(char* old_name);
};
