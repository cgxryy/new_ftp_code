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
#include <functional>

#include "package.h"

using namespace::std;

class client_data
{
	public 
		:
		int data_fd;
		int cmd_fd;
		//cmd
		sockaddr_in address_cmd;
		socklen_t addr_length_cmd;
		//data
		sockaddr_in address_data;
		socklen_t addr_length_data;
// 		old way to find the function 
// 		unordered_map<int, bool (client_data::*)(void)> fun_list;
		unordered_map<int, function<bool(void)>> fun_list;
		
		buf_data package;
		/*
		 * get
		 * put
		 * dir...
		 * 通过键值哈希——unordered_map就是泛型哈希 
		 * 通过下面定义可以直接通过下标找到对应处理函数
		 * */
		client_data()
		{
			addr_length_cmd = sizeof(address_cmd);
			addr_length_data = sizeof(address_data);
			
			flag_first_acp = false;
// old way to find the function	
//			fun_list[GET_NUMBER] = &client_data::get;
//			fun_list[PUT_NUMBER] = &client_data::put;
//			fun_list[DIR_NUMBER] = &client_data::dir;
	
			fun_list[GET_NUMBER] = [&]() {  
				return this->get();
			};
			fun_list[PUT_NUMBER] = [&]() {
				return this->put();
			};
			fun_list[DIR_NUMBER] = [&]() {
				return this->dir();
			};

		}
		bool judge_buf();

	private
		:
		//由字符串直接映射函数体
		bool flag_first_acp;
		
		bool get();
		bool put();
		bool dir();
};
/*
class buf_tool
{
	public 
		:
		buf_tool()
		{
		}
		bool extract_cmd(char* start, int* length);
		char* skip(char* start);
};
*/
class server_init
{
	private
		:
		struct ifaddrs* ifAddrStruct; 	//获取ip结构体
		void * tmpAddrPtr; 		//临时指针
		int port_cmd; 			//命令端口
		int port_data; 			//数据端口

		char ip[INET_ADDRSTRLEN]; 	//本机有线ip
		char ip_w[INET_ADDRSTRLEN]; 	//本机无线ip

		int setnonblocking(int fd); 	//设置套接字无阻塞
		char* get_ip(bool is_wifi); 	//获取本机ip
			
	public 
		:
		sockaddr_in cmd_address; 	//服务器地址结构体(传命令)
		sockaddr_in data_address; 	//服务器地址结构体(传数据)
		server_init();
		int  init_fd_addr(bool is_wifi, bool is_cmd); //设置服务器套接字并绑定监听 产生一切都就绪的文件描述符
		void addfd(int epollfd, int fd);//在epoll中添加描述符
};
