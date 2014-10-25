/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月15日 20时20分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reazon (Changgongxiaorong), cgxryy@gmail.com
 *        Company:  Class 1203 of NetWork Engineering
 *
 * =====================================================================================
 */
#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>

#include <unordered_map>
#include <string>
using namespace::std;

const int MAX_BUF_SIZE = 1024;
const int MAX_EVENT_SIZE = 1024;
const int MAX_USERS = 1024;
const int LISTEN_QUEUE_LEN = 5;

//maybe no_use
/*
const int GET_NUMBER = 0;
const int PUT_NUMBER = 1;
const int DIR_NUMBER = 2;
*/

const char* GET = "get";
const char* PUT = "put";
const char* DIR = "dir";

//代替上述写法
class client_data
{
	public 
		:
		sockaddr_in address;
		socklen_t addr_length;
		char* write;
		char buf[MAX_BUF_SIZE];
		int data_fd;
		int cmd_fd;
		/*
		 * get
		 * put
		 * dir...
		 * 通过字符串哈希——unordered_map就是泛型哈希 
		 * 通过下面定义可以直接通过下标找到对应处理函数
		 * */
		client_data()
		{
			addr_length = sizeof(address);
			
			fun_list[GET] = &client_data::get;	
			fun_list[PUT] = &client_data::put;
			fun_list[DIR] = &client_data::dir;
		}
		bool judge_buf();

	private
		:
		//由字符串直接映射函数体
		unordered_map<string, bool (client_data::*)(void)> fun_list;
		bool get();
		bool put();
		bool dir();
};

class server_init
{
	private
		:
		struct ifaddrs* ifAddrStruct; 	//获取ip结构体
		void * tmpAddrPtr; 		//临时指针
		sockaddr_in cmd_address; 	//服务器地址结构体(传命令)
		sockaddr_in data_address; 	//服务器地址结构体(传数据)
		int port_cmd; 			//命令端口
		int port_data; 			//数据端口

		char ip[INET_ADDRSTRLEN]; 	//本机有线ip
		char ip_w[INET_ADDRSTRLEN]; 	//本机无线ip

		int setnonblocking(int fd); 	//设置套接字无阻塞
		char* get_ip(bool is_wifi); 	//获取本机ip
			
	public 
		:
		server_init();
		int  init_fd_addr(bool is_wifi, bool is_cmd); //设置服务器套接字并绑定监听 产生一切都就绪的文件描述符
		void addfd(int epollfd, int fd);//在epoll中添加描述符
};

