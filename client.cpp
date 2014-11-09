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
#include <dirent.h>

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
	if (!flag_first_con)
	{
		if (connect(cmd_fd, (struct sockaddr*)&cmd_address, sizeof(cmd_address)) < 0)
		{
			cout << "conection failed" << endl;
			close(cmd_fd);
			return false;
		}
	}

	char buf_str[MAX_BUF_SIZE];
//测试代码
//	temp_package.type = PUT_NUMBER;
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
	
	if (ret != MAX_BUF_SIZE)
	{
		cout << "recv error not MAX_BUF_SIZE" << endl;
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
	
	if (!flag_first_con)
	{
		if (connect(data_fd, (struct sockaddr*)&data_address, sizeof(data_address)) < 0)
		{
			cout << "conection failed" << endl;
			close(data_fd);
			return false;
		}
		flag_first_con = true;
	}
	
	fun_list[package_recv.type]();
	
	return true;
}

bool client_init::get()
{
/*
 * 预留一个类给文件文件管理
 * */
	mkdir("../ftp_download", 0777);
	//保留程序路径，进入下载路径
	char now_path[100];
	getcwd(now_path, 100);
	int ret = chdir("../ftp_download");
	if (ret < 0)
	{
		cout << "chdir error " << strerror(errno) << endl;
	}

	cout << temp_package.buf<< endl;
	cout << basename(temp_package.buf)<< endl;
	//创建文件准备写入
	int file_fd = creat(basename(temp_package.buf), 0666);
	if (file_fd == -1)
	{
		cout << "open error" << strerror(errno) << endl;
		return false;
	}

	//先接收第一个包，里面有总长度
	buf_data package_file;
	char buf_str[MAX_BUF_SIZE];
	if (recv(data_fd, buf_str, sizeof(buf_str), MSG_WAITALL) != 1024)
	{
		cout << "doesn'n recv the sum_length" << endl;
		return false;
	}
	memcpy(&package_file, buf_str, sizeof(buf_str));
	sum_len = package_file.sum;
	download_len = 0;
	//开始循环收数据写数据
	do
	{
		int ret = recv(data_fd, buf_str, sizeof(buf_str), MSG_WAITALL);
		//接收失败
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}
		//接收不够一个包
		if (ret != MAX_BUF_SIZE)
		{
			cout << "recv data not full MAX_BUF_SIZE error"<< endl;
			return false;
		}
		cout << "recv: " << ret << endl;
		
		//读取缓存区，按有效长度读数据
		memcpy(&package_file, buf_str, sizeof(buf_str));
		ret = write(file_fd, package_file.buf, package_file.length);
		if (ret < 0)
		{
			cout << strerror(errno) << endl;
			return false;
		}
		cout << "write: " << ret << endl;
		download_len += ret;
		
		cout << "\t\t\t\t" << download_len/sum_len*100 << "%..........." << endl;
		//当监测到结束包时退出循环
		if (package_file.end_flag)
		{
			cout << "file has download completely..." << endl;
			break;
		}
	}while(1);

	chdir(now_path);
	return true;
}

bool client_init::put()
{
	int ret;

	//保留当前路径，用于传输完，恢复
	char now_path[MAX_BUFFER];
	getcwd(now_path, sizeof(now_path));

	//打开文件，准备读取
	int file_fd = open64(temp_package.buf, O_RDWR, 0644);
	cout << "package.buf:   " << temp_package.buf << endl;
	if (file_fd < 0)
	{
		cout << strerror(errno) << endl;
		return false;
	}

	//发送第一个包，包含总数据
	buf_data package_send;
	char send_buf[MAX_BUF_SIZE];
	struct stat64 file_mode;
	stat64(temp_package.buf, &file_mode);
	sum_len = file_mode.st_size;
	upload_len = 0;

	while (1)
	{
		package_send.clear();
		ret = read(file_fd, package_send.buf, sizeof(package_send.buf));
		if (ret <= 0)
		{
			package_send.clear();
			package_send.type = PUT_NUMBER;
			package_send.length = 0;
		      	package_send.end_flag = true;
			memcpy(send_buf, &package_send, sizeof(send_buf));
			send(data_fd, send_buf, sizeof(send_buf), 0);
			cout << "all send~~"<< endl;
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
		upload_len += ret;

		if (upload_len-sum_len < 0)
		      cout << "total :" << upload_len/sum_len*100 << "%......." << endl;
		else
		      cout << "total :" << 100 << "%......." << endl;
	}

	//恢复路径
	chdir(now_path);

	cout << "put completely..." << endl;

	if(package_send.end_flag)
	      return true;
	return false;
}

bool client_init::dir()
{
	//打开文件夹
	//读取文件名
	//是目录加斜杠
	//如果是结束发结束标志
	char recv_buf[MAX_BUF_SIZE];
	buf_data recv_package;
	vector<string> try_list;
	while (1)
	{
		int ret = recv(data_fd, recv_buf, sizeof(recv_buf), MSG_WAITALL);
		if (ret != MAX_BUF_SIZE)
		{
			cout << "something wrong..." << endl;
			return false;
		}
		memcpy(&recv_package, recv_buf, sizeof(recv_buf));
		tool_str::get_file_name(try_list, recv_package);

		if (recv_package.end_flag)
		{
			cout << "dir recv completely.." << endl;
			break;
		}
	}

	vector<string>::iterator iter;
	for ( iter = try_list.begin(); iter != try_list.end(); ++iter)
	{
		cout << *iter << endl;
	}

	return true;
}



int main(int argc, char* argv[])
{
	//************
	//解析字符串分析命令
	//************
	//初期代码从命令行获取
	//之后从接口类中获得
	client_init client;
	if (argc != 4)
	{
	//	cout << "usage: "<< argv[0] << ' ' << "server_ip" << endl;
//测试代码
		cout << "usage: "<< argv[0] << ' ' << "server_ip " << "filename" << "cmd"<< endl;
//测试代码
		return 1;
	}
	strncpy(client.target_ip, argv[1], strlen(argv[1]));
	client.target_ip[strlen(argv[1])] = '\0';
	

	if (strncmp(argv[3], "get", 3) == 0)
	{
		client.temp_package.type = GET_NUMBER;
	}
	else if (strncmp(argv[3], "put", 3) == 0)
	{
		client.temp_package.type = PUT_NUMBER;
	}
	else if (strncmp(argv[3], "dir", 3) == 0)
	{
		client.temp_package.type = DIR_NUMBER;
	}


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

