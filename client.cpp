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

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "client.h"

extern int errno;

int client_init::init_addr(bool is_cmd)
{
	sockaddr_in* address;
	int port;
	if (is_cmd)
	{
		port = port_cmd;
		address = &cmd_address;
	}
	else 
	{
		port = port_data;
		address = &data_address;
	}

	//服务器ip时用户输入的
	const char* ip = target_ip;
	bzero(address, sizeof(cmd_address));
	address->sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address->sin_addr);
	address->sin_port = htons(port);

	int connfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(connfd >= 0);

	return connfd;
}

int main(int argc, char* argv[])
{
	//解析字符串分析命令
	//初期代码从命令行获取
	//之后从接口类中获得
	client_init client;
	if (argc == 2)
	{
		cout << "usage: "<< argv[0] << ' ' << "server_ip" << endl;
		return 1;
	}
	strncpy(client.target_ip, argv[1]);
	
	//地址端口什么之类的初始化
	client.cmd_fd = client.init_addr(true);
	client.data_fd = client.init_addr(false);

	if (connect(client.cmd_fd, (struct sockaddr*)&client.cmd_address, sizeof(client.cmd_address)) < 0)
	{
		cout << "conection failed" << endl;
		close(client.cmd_fd);
		return 1;
	}
	//连接成功后开始进入功能函数
	

	if (connect(client.data_fd, (struct sockaddr*)&client.data_address, sizeof(client.data_address)) < 0)
	{
		cout << "connection failed" << endl;
	}
	
	

	//解析命令后直接送到对应函数中包装送出
	
	//
	
	//
	
	return 0;
}

