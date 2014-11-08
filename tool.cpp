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
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/epoll.h>
#include <fcntl.h>

#include "tool.h"

using namespace::std;

int tool_sockfd::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void tool_sockfd::addfd(int epollfd, int fd, bool oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLET | EPOLLIN;
	if (oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void tool_sockfd::reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

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

void tool_str::get_file_name(vector<string> &name_container, buf_data &name_package)
{
	stringstream ss(name_package.buf);

	string sub_str;
	while (getline(ss, sub_str, ' '))
	{
		name_container.push_back(sub_str);
	}
}

