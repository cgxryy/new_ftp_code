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
#include <sys/stat.h>

#include "client.h"
#include "tool.h"
extern int errno;

#include <iostream>

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

bool client_init::judge_cmd()
{
	if (connect(cmd_fd, (struct sockaddr*)&cmd_address, sizeof(cmd_address)) < 0)
	{
		cout << "conection failed" << endl;
		close(cmd_fd);
		return false;
	}

	char buf_str[1024];
//测试代码
	temp_package.type = GET_NUMBER;
//测试代码
	temp_package.end_flag = true;
	memcpy(buf_str, &temp_package, sizeof(temp_package));
	
	cout << "temp_package.buf contains..." << endl;
	cout << temp_package.buf << endl;
	int ret = send(cmd_fd, buf_str, sizeof(temp_package), 0);
	if (ret < 0)
	{
		cout << "send error" << strerror(errno) << endl;
		return false;
	}
	cout << "first package send"<< endl;
//	ret = recv(cmd_fd, buf_str, sizeof(buf_str), MSG_WAITALL);
//测试代码
	ret = recv(cmd_fd, buf_str, sizeof(buf_str), 0);
//测试代码
	if (ret < 0)
	{
		cout << strerror(errno) << endl;
		return false;
	}
	
	if (ret != 1024)
	{
		cout << "recv error not 1024" << endl;
		return false;
	}

	buf_data package_recv;
	memcpy(&package_recv, buf_str, sizeof(buf_str));
	if (!package_recv.ack_flag)
	{
		cout << "recv not the ack package, someone set the wrong package" << endl;
	}

	if(fun_list[package_recv.type] == NULL)
	{
		cout << "no function to deal with it,something wrong" << endl;
		return false;
	}
	fun_list[package_recv.type]();
	
	return true;
}

bool client_init::get()
{
	if (connect(data_fd, (struct sockaddr*)&data_address, sizeof(data_address)) < 0)
	{
		cout << "conection failed" << endl;
		close(data_fd);
		return false;
	}
/*
 * 预留一个类给文件文件管理
 *
 * */
	mkdir("../ftp_download", 0777);
	
	buf_data package_file;
	char buf_str[1024];

	char now_path[100];
	getcwd(now_path, 100);
	int ret = chdir("../ftp_download");
	if (ret < 0)
	{
		cout << "chdir error " << strerror(errno) << endl;
	}

	cout << temp_package.buf<< endl;
	cout << basename(temp_package.buf)<< endl;
	int file_fd = creat(basename(temp_package.buf), 0666);
	if (file_fd == -1)
	{
		cout << "open error" << strerror(errno) << endl;
		return false;
	}

	do
	{
		int ret = recv(cmd_fd, buf_str, sizeof(buf_str), MSG_WAITALL);
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}
		if (ret != 1024)
		{
			cout << "recv data not full 1024 error"<< endl;
			return false;
		}

		memcpy(&package_file, buf_str, sizeof(buf_str));
		ret = write(file_fd, package_file.buf, package_file.length);
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}

		if (package_file.end_flag)
		{
			cout << "file has download completely..." << endl;
			break;
		}
	//测试代码
		cout << "recv data: " << ret << endl;
	//测试代码	
	}while(1);

	chdir(now_path);
	return true;
}

bool client_init::put()
{
	return false;
}

bool client_init::dir()
{
	return false;
}



int main(int argc, char* argv[])
{
	//************
	//解析字符串分析命令
	//************
	//初期代码从命令行获取
	//之后从接口类中获得
	client_init client;
	if (argc != 3)
	{
	//	cout << "usage: "<< argv[0] << ' ' << "server_ip" << endl;
//测试代码
		cout << "usage: "<< argv[0] << ' ' << "server_ip " << "filename" << endl;
//测试代码
		return 1;
	}
	strncpy(client.target_ip, argv[1], strlen(argv[1]));
	client.target_ip[strlen(argv[1])] = '\0';
	
	//地址端口什么之类的初始化
	client.cmd_fd = client.init_addr(true);
	client.data_fd = client.init_addr(false);
	
//测试代码
	strncpy(client.temp_package.buf, argv[2], strlen(argv[2]));
	client.temp_package.buf[strlen(argv[2])] = '\0';
	client.temp_package.length = strlen(argv[2]);
//测试代码
	
	//连接命令端口
	//允许不断输入命令并解析
//	while(1)
//	{
		//解析成功后开始进入功能函数
		client.judge_cmd();
//	}

	
	
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

