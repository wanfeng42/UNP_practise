/*
  功能：本文件包含了所需要的头文件,并声明了服务器和客户端的公用宏与函数
  更新日期：2018-6-15
  作者：万锋
*/

#ifndef _HEAD_H_
#define _HEAD_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>

#define MAXBUFLEN 1024 //最大缓冲区长度
#define MAXSTRLEN 128  //用户名和密码最大长度
#define MAXSESS 16 //最大sess长度

int Readline(int connfd, char *buf);
int find_sign(char s, char *str);

#endif