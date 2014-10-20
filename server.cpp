/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月14日 17时16分25秒
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
#include <stdlib.h>

#include <fcntl.h>
#include <sys/epoll.h>
#include "server.h"

#include <arpa/inet.h>
#include <unordered_map>

#include <iostream>

using namespace::std;

int server_init::init_fd_addr(bool is_wifi, bool is_cmd)
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
	const char* ip = get_ip(is_wifi);
	assert(ip != NULL);

	int ret = 0;
	bzero(address, sizeof(cmd_address));
	address->sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address->sin_addr);
	address->sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	ret = bind(listenfd, (struct sockaddr*)address, sizeof(cmd_address));
	assert(ret != -1);

	ret = listen(listenfd, LISTEN_QUEUE_LEN);
	assert(ret != -1);

	return listenfd;
}

int server_init::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void server_init::addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLET | EPOLLIN;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

server_init::server_init()
{
	port_cmd = 12344;
	port_data = 12345;
}

char* server_init::get_ip(bool is_wifi)
{
	memset(ip, '\0', INET_ADDRSTRLEN);
	memset(ip_w, '\0', INET_ADDRSTRLEN);
	getifaddrs(&ifAddrStruct);
	while (ifAddrStruct != NULL)
	{
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET) 
		{
			if (strncmp(ifAddrStruct->ifa_name, "p3p1", 4) == 0)
			{
				tmpAddrPtr = &((struct sockaddr_in*)ifAddrStruct->ifa_addr)->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
			}
			else if (strncmp(ifAddrStruct->ifa_name, "wlp3s0", 6) == 0)
			{
				tmpAddrPtr = &((struct sockaddr_in*)ifAddrStruct->ifa_addr)->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, ip_w, INET_ADDRSTRLEN);
			}
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
	if (is_wifi)
	{
		if (ip_w[0] != '\0')
		      return ip_w;
		else if (ip[0] != '\0')
		      return ip;
		else return NULL;
	}
	else 
	{
		if (ip[0] != '\0')
			return ip;
		else if (ip_w[0] != '\0')
		      return ip_w;
		else return NULL;
	}

	cout << ip << endl;
	cout << ip_w << endl;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("usage: %s y/n(启用无线，否则有线)\n", basename(argv[0]));
		return 1;
	}

	bool is_wifi;
	if (atoi(argv[1]) == 'y')
	      is_wifi = true;
	else is_wifi = false;

	server_init* server = new server_init();
	
	int listenfd_cmd = server->init_fd_addr(is_wifi, true);
	int listenfd_data = server->init_fd_addr(is_wifi, false);

	//把客户连接过来的套接字和地址结构体对应起来
	unordered_map<int, client_data*> map_fd_addr;
	
	//epoll初始化
	epoll_event* events = new epoll_event[MAX_EVENT_SIZE];
	int epollfd = epoll_create(MAX_USERS);
	assert(epollfd != -1);

	server->addfd(epollfd, listenfd_cmd);
	server->addfd(epollfd, listenfd_data);

	while (1)
	{
		int number = epoll_wait(epollfd, events, MAX_EVENT_SIZE, -1);
		if (number < 0)
		{
			cout << "epoll failure" << endl;
			break;
		}

		for ( int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd_cmd)
			{
				
			}
		}
	}

	return 0;
}

