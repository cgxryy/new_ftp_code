#pragma once
#include <sys/types.h>

const int MAX_BUF_SIZE = 1024;
const int MAX_EVENT_SIZE = 1024;
const int MAX_USERS = 1024;
const int LISTEN_QUEUE_LEN = 5;

//maybe no_use
const int IP_NUMBER = 1;

const int GET_NUMBER = 2;
const int PUT_NUMBER = 3;
const int DIR_NUMBER = 4;

const int MAX_NUMBER = 4;

//包中数据大小 
const int MAX_BUFFER = 1014;

//自定义包结构
struct buf_data
{
	int32_t type; 	//包的类型 get put dir 
	int32_t length; //包的长度 0~1014
	bool 	end_flag;
	bool 	ack_flag;
	char 	buf[MAX_BUFFER]; //1024-4-4-2 = 1014

	buf_data()
	{
		type = 0;
		length = 0;
		end_flag = false;
		ack_flag = false;
		memset(buf, '\0', sizeof(buf));
	}
	void clear()
	{
		type = 0;
		length = 0;
		end_flag = false;
		ack_flag = false;
		memset(buf, '\0', sizeof(buf));
	}
};
