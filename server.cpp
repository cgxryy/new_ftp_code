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
#include "tool.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <dirent.h>

#include <iostream>
#include <vector>

extern int errno;

using namespace::std;

int server_init::init_fd_addr(bool is_wifi, bool is_cmd)
{
	sockaddr_in* address;
	int* port;
	if (is_cmd)
	{
		port = &port_cmd;
		address = &cmd_address;
	}
	else 
	{   
		port = &port_data;
		address = &data_address;
	}
	assert(ip != NULL);

	int ret = 0;
	bzero(address, sizeof(cmd_address));
	address->sin_family = AF_INET;
//	const char* target_ip = get_ip(is_wifi);
//测试代码
	const char* target_ip = "127.0.0.1";
//测试代码
	cout << target_ip << endl;

	inet_pton(AF_INET, target_ip, &address->sin_addr);
	address->sin_port = htons(*port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	ret = bind(listenfd, (struct sockaddr*)address, sizeof(cmd_address));
	assert(ret != -1);

	ret = listen(listenfd, LISTEN_QUEUE_LEN);
	assert(ret != -1);

	return listenfd;
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
	//the code below shows 'is_wifi' is just a suggestion not a commamd
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

bool client_data::judge_buf()
{
	char buf_str[MAX_BUF_SIZE];
//测试代码
	int ret = recv(cmd_fd, buf_str, sizeof(buf_str), 0);
//测试代码
	memcpy(&package, buf_str, sizeof(package));
	if (ret < 0)
	{
		cout << "judge_buf" << strerror(errno) << endl;
		return false;
	}
	cout << "first package.buf...  " << package.buf << endl;
	//类型不在范围内，为恶意包丢弃
	if (package.type > MAX_NUMBER)
	      return false;
	//回复一个收到确认包
	//回复不要用类中的，get会用
	buf_data package_ack;
	
	package_ack.clear();
	package_ack.type = package.type;
	package_ack.length = 0;
	package_ack.ack_flag = true;
	memcpy(buf_str, &package_ack, sizeof(package_ack));
	ret = send(cmd_fd, buf_str, sizeof(buf_str), 0);

	if (!flag_first_acp)
	{
		this->data_fd = accept(this->data_fd, (struct sockaddr*)&(this->address_data), &(this->addr_length_data));
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}
		else 
		{
			cout << "accept return " << this->data_fd << endl;
		}
		flag_first_acp = true;
	}

	return true;
}

bool client_data::get()
{
	int ret;

	//保留当前路径，用于传输完，恢复
	char now_path[MAX_BUFFER];
	getcwd(now_path, sizeof(now_path));

	//预留一个类给处理路径， 确保文件都在一个指定文件夹不会越权
	// "/home" -> "./home"   去掉".."
	cout << "before change:  " << package.buf << endl;
	char* change;
	if ((change = tool_str::path_change(package.buf)) == NULL)
	{
		cout << "path change failed..." << endl;
		return false;
	}

	cout << change << endl;
	cout << "package length: " << package.length << endl;
	cout << change << endl;

	
	mkdir("../ftp_source", 0777);
	ret = chdir("../ftp_source");
	if (ret < 0)
	{
		cout << "into the ftp_source failed" << endl;
		return false;
	}
	else 
		cout << "now at ftp_source.." << endl;

	//打开文件，准备读取
	int file_fd = open(package.buf, O_RDWR, 0644);
	cout << "package.buf:   " << package.buf << endl;
	if (file_fd < 0)
	{
		cout << strerror(errno) << endl;
		return false;
	}

	buf_data package_send;
	char send_buf[MAX_BUF_SIZE];
	//获取文件总大小发送(显示进度条)第一个包
	struct stat64 file_mode64;
	stat64(package.buf, &file_mode64);
	package_send.sum = file_mode64.st_size;
	memcpy(send_buf, &package_send, sizeof(send_buf));
	if (send(data_fd, send_buf, sizeof(send_buf), 0) < 0)
	{
		cout << "something wrong with send, line:" << __LINE__ << endl;
		return false;
	}

	//开始发送正式数据
	while (1)
	{
		package_send.clear();
		ret = read(file_fd, package_send.buf, sizeof(package_send.buf));
		if (ret <= 0)
		{
			package_send.clear();
			package_send.type = GET_NUMBER;
			package_send.length = 0;
		      	package_send.end_flag = true;
			memcpy(send_buf, &package_send, sizeof(send_buf));
			cout << "package send all..." << endl;
			if (send(data_fd, send_buf, sizeof(send_buf), 0) < 0)
			      cout << strerror(errno) << endl;
			break;
		}
		package_send.length = ret;
//测试代码
		cout << "read " << ret << endl;
//测试代码
		memcpy(send_buf, &package_send, sizeof(send_buf));
		ret = send(data_fd, send_buf, sizeof(send_buf), 0);
//测试代码
		cout << "send " << ret << endl;
//测试代码
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}
	
	}

	//恢复路径
	chdir(now_path);

	if(package_send.end_flag)
	      return true;

	return false;
}

bool client_data::put()
{
	//解析后面的文件路径系统
	//只要文件名
	mkdir("../ftp_source", 0777);
	
	//path_extract_name()
	//打开新文件准备写入
	char now_path[100];
	getcwd(now_path, 100);
	int ret = chdir("../ftp_source");
	if (ret < 0)
	{
		cout << "chdir error" << strerror(errno) << endl;
	}

	int file_fd = creat(basename(package.buf), 0666);
	if (file_fd < 0)
	{
		string name_copy;
		name_copy = tool_str::copy_file(basename(package.buf));
		file_fd = creat(name_copy.c_str(), 0666);
		if (name_copy.empty())
		{
			cout << "open error " << strerror(errno) << endl;
			return false;
		}
	}

	//不断接收数据
	buf_data package_file;
	char buf_str[MAX_BUF_SIZE];
//	char* ptr = buf_str;
//	char* end = &buf_str[MAX_BUF_SIZE-1];
	do
	{
//		int ret = read(data_fd, ptr, end-ptr+1);
		int ret = recv(data_fd, buf_str, sizeof(buf_str), MSG_WAITALL);
//		if (ret < 0 && errno != EAGAIN)
		if (ret != 1024)
		{
			cout << "ret < 0 " << __LINE__ << endl;
			cout << strerror(errno)  << __LINE__ << endl;
			return false;
		}
		cout << "1024开始写入" << endl;
		memcpy(&package_file, buf_str, sizeof(buf_str));
		ret = write(file_fd, package_file.buf, package_file.length);
		if (ret < 0)
		{
			cout << "ret < 0 " << __LINE__ << endl;
			cout << strerror(errno)  << __LINE__ << endl;
			return false;
		}
		cout << "write" << ret << endl;
		
		if (package_file.end_flag)
		{
			cout << "file has be put completely..." << endl;
			break;
		}
//		}
	}while (1);

	chdir(now_path);

	if (package_file.end_flag)
		return true;
	return false;
}

bool client_data::dir()
{
	//解析文件路径系统
	//只要所有文件名
	
	DIR *dir;
	struct dirent* ptr;
	
	char now_path[MAX_BUF_SIZE];
	getcwd(now_path, MAX_BUF_SIZE);
	
	//before enter the dir ,it need to do some check
	bool dir_flag = false;
	if (strncmp(package.buf, "/", 2) == 0)
	{
		dir_flag = true;
	}
	string path_change = "../ftp_source";
	path_change += package.buf;

	if (chdir(path_change.c_str()) < 0)
	{
		cout << "chdir error...358" << endl;
		return false;
	}

	if ((dir = opendir(path_change.c_str())) == NULL)
	{
		cout << strerror(errno) << endl;
		return false;
	}
	

	buf_data send_package;//发送包
	int length = 0;//有效字长
	char send_buf[MAX_BUF_SIZE];//发送缓冲区
	string send_string;//发送的文件名
	string file_string;//完整文件名
	struct stat file_stat;//文件状态
	while ((ptr = readdir(dir)) != NULL)
	{
		if (strlen(ptr->d_name) > sizeof(send_package.buf))
		{
			cout << "strlen(ptr->d_name)" << strlen(ptr->d_name) << endl;
			cout << "sizeof(send_package.buf)" << sizeof(send_package.buf) << endl;
			cout << "file name too long...error" << endl;
			return false;
		}

		if (dir_flag)
		{
			string del_name = ptr->d_name;
			if (del_name.compare(".") == 0 || del_name.compare("..") == 0)
			{
				continue;
			}
		}

		if (length + strlen(ptr->d_name) > sizeof(send_package.buf))
		{
			memcpy(send_buf, &send_package, sizeof(send_buf));
			send(data_fd, send_buf, sizeof(send_buf), 0);
			send_package.clear();
			length = 0;
			continue;
		}
		
		//judge file or dir
		file_string.clear();
		file_string += package.buf;
		file_string += "/";
		file_string += ptr->d_name;
		lstat(file_string.c_str(), &file_stat);
		if (S_ISDIR(file_stat.st_mode))
		{
			length += strlen(ptr->d_name) + 2;
			send_string += ptr->d_name;
			send_string += "/ ";
		}
		else 
		{
			length += strlen(ptr->d_name) + 1;
			send_string += ptr->d_name;
			send_string += " ";
		}
	}
	memcpy(send_package.buf, send_string.c_str(), sizeof(send_package.buf));
	send_package.end_flag = true;
	
	memcpy(send_buf, &send_package, sizeof(send_buf));
	send(data_fd, send_buf, sizeof(send_buf), 0);

	chdir(now_path);

	return true;
}

//pthread function
void* work_function(void* arg)
{
	//判断字符串，哈希找到函数并执行
	pthread_detach(pthread_self());
	client_data* client = (client_data*)arg;
	client->fun_list[client->package.type]();
//	client->((*client->fun_list.find(client->package.type))();

	return NULL;
}

//signal function
void handler_sigint(int signo)
{
	cout << "server is stopped by some reasons......" << endl;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("usage: %s y/n(启用无线，否则有线)\n", basename(argv[0]));
		return 1;
	}

	//*********
	//addr init
	//*********
	bool is_wifi;
	if (atoi(argv[1]) == 'y')
	      is_wifi = true;
	else is_wifi = false;

	server_init* server = new server_init();
	
	int listenfd_cmd = server->init_fd_addr(is_wifi, true);
	int listenfd_data = server->init_fd_addr(is_wifi, false);

	//connect the fd and the client_data
	unordered_map<int, client_data*> map_fd_addr;

	//*********
	//epoll init
	//*********
	epoll_event* events = new epoll_event[MAX_EVENT_SIZE];
	int epollfd = epoll_create(MAX_USERS);
	assert(epollfd != -1);

	//epoll only contains cmd_fd ,it doesn's contain data_fd
	tool_sockfd::addfd(epollfd, listenfd_cmd, true);

	signal(SIGINT, handler_sigint);

	//用来通过文件描述符找客户端结构体
	unordered_map<int, client_data*> fd_map;
	while (1)
	{
		int number = epoll_wait(epollfd, events, MAX_EVENT_SIZE, -1);
		if (number < 0)
		{
			cout << "epoll failure" << endl;
			continue;
		}
		
		for ( int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			//新的套接字
			if (sockfd == listenfd_cmd)
			{

				client_data* client = new client_data();
				client->data_fd = listenfd_data;
				//client->cmd_fd已经由listen后的变为accept后的了
				client->cmd_fd = accept(listenfd_cmd, (struct sockaddr*)&client->address_cmd, &client->addr_length_cmd);
//测试代码
				tool_sockfd::reset_oneshot(epollfd, listenfd_cmd);
				cout << "client->cmd_fd" << client->cmd_fd << endl;
//测试代码
				tool_sockfd::addfd(epollfd, client->cmd_fd, true);
				if (client->cmd_fd < 0)
				{
					printf("errno is: %s\n", strerror(errno));
					continue;
				}
				fd_map[client->cmd_fd] = client;
			}
			//命令传输较少，我们采用IO复用处理，文件传输较长，分出线程处理
			//之前的套接字，有新的数据
			else if (events[i].events & EPOLLIN)
			{
				(fd_map[sockfd]->package).clear();
				fd_map[sockfd]->judge_buf();
				pthread_t work_pthread;
				int ret = pthread_create(&work_pthread, NULL, work_function, (void*)fd_map[sockfd]);
				if (ret != 0)
				{
					cout << "create pthread error" << endl;
				}
			//	tool_sockfd::reset_oneshot(epollfd, sockfd);
			}
			else 
			{
				cout << "something else happened" << endl;
			}
		}
	}

	return 0;
}

