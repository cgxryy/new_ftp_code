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

class client_init
{
	public 
		:
		int cmd_fd;
		int data_fd;

		sockaddr_in cmd_address; 	//服务器地址结构(传命令)
		sockaddr_in data_address; 	//服务器地址结构(传数据)
	
		int port_cmd;
		int port_data;

		char target_ip[INET_ADDRSTRLEN];//目标服务器ip
		
		client_init()
		{
			strncpy(target_ip, server_ip, strlen(server_ip));
			port_cmd = 12344;
			port_data = 12345;
		}

		//返回值初始化后文件描述符
	 	int init_addr();
		bool connect_timelimit(int sockfd, struct sockaddr* address, int time, int count);
		bool judge_cmd();
		bool recv_timelimit();
	private
		:
			


};
