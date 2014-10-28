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

#include <fcntl.h>
#include <errno.h>

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

bool client_init::connect_timelimit(int sockfd, struct sockaddr* address, int time, int count)
{
	int i = count;
	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	socklen_t len = sizeof(timeout);
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	assert(ret != -1);

	while (i > 0)
	{
		if ((ret = connect(sockfd, address, sizeof(struct sockaddr))) == -1)
		{
			if (errno == EINPROGRESS)
			{
				i--;
				continue;
			}
			else 
			{
				return false;
			}
		}
		else 
		      return true;
	}
	return false;
}

bool client_init::recv_timelimit(int sockfd, void* buf, size_t len, int flag, int time)
{
	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	
	socklen_t len = sizeof(timeout);
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	assert(ret != -1);

	//这里有问题，再看看
	ret = recv(sockfd, buf, len, flag);
	if (ret == -1)
	{
		if (errno == EINPROGRESS)
		{
			recv(sockfd, buf, len, flag);
		}
	}

	return true;
}

bool judge_cmd()
{
	if (client.connect_timelimit(client.cmd_fd, (struct sockaddr*)&client.cmd_address, 3, 3) < 0)
	{
		cout << "conection failed" << endl;
		close(client.cmd_fd);
		return 1;
	}
	int ret = recvmm();	
}

int main(int argc, char* argv[])
{
	//************
	//解析字符串分析命令
	//************
	//初期代码从命令行获取
	//之后从接口类中获得
	client_init client;
	if (argc == 2)
	{
		cout << "usage: "<< argv[0] << ' ' << "server_ip" << endl;
		return 1;
	}
	strncpy(client.target_ip, argv[1], strlen(argv[1]));
	
	//地址端口什么之类的初始化
	client.cmd_fd = client.init_addr(true);
	client.data_fd = client.init_addr(false);

	//连接命令端口
	client.judge_cmd();

	
	//************
	//连接成功后开始进入功能函数
	//************
	
/* 
 * 连接数据端口是已经连上命令端口并获得恢复的情况下才进行的
	if (client.connect_timelimit(client.data_fd, (struct sockaddr*)&client.data_address, 3, 3) < 0)
	{
		cout << "conection failed" << endl;
		close(client.data_fd);
		return 1;
	}
*/	
	

	//解析命令后直接送到对应函数中包装送出
	
	//
	
	//
	
	return 0;
}

