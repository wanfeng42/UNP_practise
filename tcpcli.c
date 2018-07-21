/***
 * 功能：本文件为客户端的处理文件,实现了与服务器的通信
 * 更新日期：2018-7-4
 * 作者：万锋
***/
#include "heads.h"

int main(int argc, char **argv)
{
    int sockfd;
    int idx, idxa;
    char recvbuf[MAXBUFLEN];
    char sendbuf[MAXBUFLEN];
    char sess[MAXSESS];
    struct sockaddr_in servaddr;

    if (argc != 4)
    {
        fprintf(stderr, "%s usage : <IP> <name> <passwd>\n", argv[0]);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(49152);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    {
        perror("connect error");
    }

    sprintf(sendbuf, "LOGIN?NAME=%s&PASSWORD=%s\n", argv[2], argv[3]);
    write(sockfd, sendbuf, strlen(sendbuf));
    Readline(sockfd, recvbuf);
    close(sockfd);
    //printf("%s\n",recvbuf);
    
    if((strncmp(recvbuf, "login failed\n", sizeof(recvbuf))) == 0)
    {
         fprintf(stderr, recvbuf, argv[0]);
         return 0;
    }

    idx = find_sign('=', recvbuf) + 1;
    idxa = find_sign('\n', recvbuf);
    strncpy(sess, recvbuf + idx, idxa - idx);
    sess[idxa - idx] = 0;
    //printf("%s\n",sess);

    memset(recvbuf, 0, sizeof(recvbuf));

    printf("login success\n");


    printf("1.TIME 2.QUIT\nenter the number of the option\n");

    while (1)
    {
        int op;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
        {
            perror("connect error");
        }

        printf("> ");
        scanf("%d", &op);
        if (op == 1)
        {
            sprintf(sendbuf, "TIME?sess=%s\n", sess);
            write(sockfd, sendbuf, strlen(sendbuf));
            Readline(sockfd, recvbuf);
            printf("%s", recvbuf);
            close(sockfd);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
        else if (op == 2)
        {
            sprintf(sendbuf, "QUIT?sess=%s\n", sess);
            write(sockfd, sendbuf, strlen(sendbuf));
            Readline(sockfd, recvbuf);
            printf("%s", recvbuf);
            close(sockfd);
            break;
        }
    }
    return 1;
}