/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月15日 21时09分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reazon (Changgongxiaorong), cgxryy@gmail.com
 *        Company:  Class 1203 of NetWork Engineering
 *
 * =====================================================================================
 */
#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <string.h>
#include "package.h"

#include <unordered_map>

using namespace::std;

class find_server
{
	public
		:
	
		find_server()
		{
		}
		bool find_init();
		bool try_find();
};

class client_init
{
	public 
		:
		int cmd_fd; 		//命令套接字
		int data_fd; 		//数据套接字
	
		int port_cmd;
		int port_data;

		char target_ip[INET_ADDRSTRLEN];//目标服务器ip
	
		unordered_map<int, bool (client_init::*)(void)> fun_list;

		buf_data temp_package;
		client_init()
		{
			port_cmd = 12344;
			port_data = 12345;

			fun_list[GET_NUMBER] = &client_init::get;
			fun_list[PUT_NUMBER] = &client_init::put;
			fun_list[DIR_NUMBER] = &client_init::dir;
		}

		//返回值初始化后文件描述符
	 	int init_addr(bool is_cmd);
		bool connect_timelimit(int sockfd, struct sockaddr* address, int time, int count);
		bool judge_cmd();

	private
		:
		sockaddr_in cmd_address; 	//服务器地址结构(传命令)
		sockaddr_in data_address; 	//服务器地址结构(传数据)

		bool get(); 			//下载
		bool put(); 			//上传
		bool dir(); 			//显示目录
};
