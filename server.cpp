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

#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "server.h"

#include <arpa/inet.h>
#include <vector>
#include <pthread.h>
#include <errno.h>

#include <iostream>

extern int errno;

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
	assert(ip != NULL);

	int ret = 0;
	bzero(address, sizeof(cmd_address));
	address->sin_family = AF_INET;
	//const char* target_ip = get_ip(is_wifi);
//测试代码
	const char* target_ip = "127.0.0.1";
//测试代码

	inet_pton(AF_INET, target_ip, &address->sin_addr);
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
	setnonblocking(fd);
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

}
/*
bool buf_tool::extract_cmd(char* start, int* length)
{
	char* end_p = strchr(start, '\0');
	char* word_p = strchr(start, ' ');
	if (end_p-start <= word_p-start)
		return false;
	
	//命令长度
	int str_len = word_p - start;
	if (str_len <= 0)
	      return false;
	else *length = str_len;

	return true;
}

char* buf_tool::skip(char* start)
{
	while ((start == ' ' || start == '\t' || start || == '\n') && (start != NULL))
	      start++;

	return start;
}
*/
bool client_data::judge_buf()
{
	char buf_str[1024];
	int ret = recv(cmd_fd, buf_str, sizeof(buf_str), MSG_WAITALL);
	memcpy(&package, buf_str, sizeof(package));
	if (ret < 0)
	{
		cout << strerror(errno) << endl;
		return false;
	}

	//类型不在范围内，为恶意包丢弃
	if (package.type > MAX_NUMBER)
	      return false;
	//回复一个收到确认包
	package.clear();
	package.type = GET_NUMBER;
	package.length = 0;
	package.ack_flag = true;
	memcpy(buf_str, &package, sizeof(package));
	ret = send(cmd_fd, buf_str, sizeof(package), 0);

	//进入功能函数
	bool err_flag = fun_list[package.type];
	return err_flag;
}

bool client_data::get()
{
	int ret;
	ret = accept(this->data_fd, (struct sockaddr*)&(this->address), &(this->addr_length));
	assert(ret < 0);

	//保留当前路径，用于传输完，恢复
	char now_path[MAX_BUFFER];
	getcwd(now_path, sizeof(now_path));
//
//预留一个类给处理路径， 确保文件都在一个指定文件夹不会越权
//
	chdir(dirname(package.buf));
	cout << "now at " << dirname(package.buf) << endl;
	//打开文件，准备读取
	int file_fd = open(basename(package.buf), O_RDWR, 0644);
	if (file_fd < 0)
	{
		cout << strerror(errno) << endl;
		return false;
	}

	buf_data package_send;
	while (1)
	{
		package_send.clear();
		ret = read(file_fd, package_send.buf, sizeof(package_send.buf));
		if (ret < 0)
		{
			break;
		}
		if (ret == 0)
		      package_send.end_flag = true;
		package_send.length = ret;
	
		ret = send(data_fd, (void*)&package_send, sizeof(package_send), 0);
		if (ret < 0)
		{
			break;
		}
		if(package_send.end_flag)
		      return true;
	}


	return false;
}

bool client_data::put()
{
	//解析后面的文件路径系统
	//只要文件名
	//path_extract_name()
	//打开新文件准备写入

	return false;
}

bool client_data::dir()
{

	//解析文件路径系统
	//只要所有文件名
	return false;
}

void* work_function(void* arg)
{
	//判断字符串，哈希找到函数并执行
	client_data* client = (client_data*)arg;
	client->judge_buf();

	return NULL;
}


int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("usage: %s y/n(启用无线，否则有线)\n", basename(argv[0]));
		return 1;
	}

	//*********
	//初始化部分
	//*********
	bool is_wifi;
	if (atoi(argv[1]) == 'y')
	      is_wifi = true;
	else is_wifi = false;

	server_init* server = new server_init();
	
	int listenfd_cmd = server->init_fd_addr(is_wifi, true);
	int listenfd_data = server->init_fd_addr(is_wifi, false);

	//把客户连接过来的套接字和地址结构体对应起来
	unordered_map<int, client_data*> map_fd_addr;


	//*********
	//epoll初始化
	//*********
	epoll_event* events = new epoll_event[MAX_EVENT_SIZE];
	int epollfd = epoll_create(MAX_USERS);
	assert(epollfd != -1);

	server->addfd(epollfd, listenfd_cmd);
//	server->addfd(epollfd, listenfd_data);

	//用来通过文件描述符找客户端结构体
	unordered_map<int, client_data*> fd_map;
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
			//新的套接字
			if (sockfd == listenfd_cmd)
			{

				client_data* client = new client_data();
				client->cmd_fd = accept(listenfd_cmd, (struct sockaddr*)&client->address, &client->addr_length);
				if (client->cmd_fd < 0)
				{
					printf("errno is: %s\n", strerror(errno));
					continue;
				}
				fd_map[client->cmd_fd] = client;
				//根据接收到的命令处理
				
				//把传数据的套接字传过去方便传输
				//switch ()
				//get
				//put
				//dir
				//命令传输较少，我们采用IO复用处理，文件传输较长，分出线程处理
				pthread_t work_pthread;
				int ret = pthread_create(&work_pthread, NULL, work_function, (void*)&client);
				assert(ret < 0);
			}
			//之前的套接字，有新的数据
			else if (events[i].events & EPOLLIN)
			{
				(fd_map[sockfd]->package).clear();
				char recv_buf[1024];
				int ret = recv(sockfd, recv_buf, sizeof((fd_map[sockfd])->package), MSG_WAITALL);
				memcpy(&(fd_map[sockfd])->package, recv_buf, sizeof(recv_buf));
				if (ret < 0)
				{
					cout << strerror(errno) << endl;
					exit(1);
				}
				pthread_t work_pthread;
				ret = pthread_create(&work_pthread, NULL, work_function, (void*)fd_map[sockfd]);
				assert(ret < 0);
			}
			else 
			{
				cout << "进入非创建无数据，可能断开状态" << endl;
			}
		}
	}

	return 0;
}

