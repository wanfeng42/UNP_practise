/*
  功能：本文件定义服务器和客户端的公用函数
  更新日期：2018-6-15
  作者：万锋
*/

#include "heads.h"

/********
 * 从字符串str中寻找第一次出现的指定字符s
 * @param s 指定的字符
 * @param str 字符串指针
 * @return <0表示未找到,否则表示该字符在str中第一次出现的索引
 *********/
int find_sign(char s, char *str)
{
    if((str == NULL) || (*str == 0))
        return -1;
    int i = 0;
    while ((str[i] != 0) || (i < MAXSTRLEN))
    {
        if (str[i] == s)
            return i;
        i++;
    }
    return -1;
}

/********
 * 从文件描述符读一行
 * @param connfd 文件描述符
 * @param buf 缓冲区
 * @return <0表示出错,否则返回读到的字符数
 *********/
int Readline(int connfd, char *buf)
{
    int rc, n;
    char *ptr, c;
    ptr = buf;
    for (n = 1, rc = 0; n < MAXBUFLEN; n++)
    {
        if ((rc = read(connfd, &c, 1)) == 1)
        {
            *ptr = c;
            ptr++;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            *ptr = '\0';
            return (n - 1);
        }
        else if (rc == -1)
        {
            return -1;
        }
    }
    *ptr = '\0';
    return n;
}