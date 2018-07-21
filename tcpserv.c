/***
 * 功能：本文件为服务器的处理文件,实现了读取配置文件,响应客户端的请求
 * 更新日期：2018-6-15
 * 作者：万锋
***/
#include "heads.h"

#define IPCPATH "/home"
#define IPCID 42

static int s_usernum; //用户个数

/********
 * 用户链表结点,存储用户名,密码
 *******/
struct user_list
{
    char name[MAXSTRLEN];
    char passwd[MAXSTRLEN];
    struct user_list *next;
};

/********
 * 存储从文件读取的配置信息
 *******/
struct config_info
{
    int port;
    struct user_list *head;
};

/********
 * 存储注册的session
********/
struct session
{
    char name[MAXSTRLEN];
    char sess[MAXSESS];
};

/********
 * 读取配置文件
 * @param path 配置文件路径
 * @param info 配置信息的储存目的
 *********/
int init(const char *path, struct config_info *info)
{
    char buf[MAXBUFLEN];
    FILE *configfp;
    struct user_list *head = NULL, *ptr;
    int idx = 0; //接受find_sign函数返回的下标

    if ((configfp = fopen(path, "r")) == NULL)
    {
        perror("init: can't open config file");
        exit(-1);
    }
    while (fgets(buf, MAXBUFLEN, configfp) != NULL)
    {
        if (buf[strlen(buf) - 1 == '\n'])
            buf[strlen(buf) - 1] = '\0';

        idx = find_sign('=', buf);
        if (idx <= 0)
            continue;

        if (strncmp(buf, "port", idx) == 0)
        {
            info->port = atoi(buf + idx + 1);
        }
        if (strncmp(buf, "name", idx) == 0)
        {
            ptr = (struct user_list *)malloc(sizeof(struct user_list));
            s_usernum++;
            strcpy(ptr->name, buf + idx + 1);
            fgets(buf, MAXBUFLEN, configfp);
            if (buf[strlen(buf) - 1 == '\n'])
                buf[strlen(buf) - 1] = '\0';

            idx = find_sign('=', buf);
            if (idx <= 0)
            {
                strcpy(ptr->passwd, "");
            }
            else
                strcpy(ptr->passwd, buf + idx + 1);
            ptr->next = head;
            head = ptr;
        }
    }
    info->head = head;
    fclose(configfp);
    return 0;
}

/********
 * 确认用户登录信息是否匹配
 * @param name 用户名
 * @param passwd 密码
 * @parm info 配置文件信息(含用户名和密码)
 *********/
int login_confirm(char *name, char *passwd, struct config_info *info)
{
    struct user_list *head;
    head = info->head;
    while (head != NULL)
    {
        if ((strncmp(name, head->name, strlen(head->name))) == 0)
        {
            if ((strncmp(passwd, head->passwd, strlen(head->passwd))) == 0) //匹配
            {
                return 1;
            }
            return 0;
        }
        head = head->next;
    }
    return 0;
}

/********
 * 生成sess
 * @param str 存储生成的sess
 * @param name sess对应的用户名
 * @parm shmaddr 共享内存地址
 *********/
int session_generate(char *str, char *name, struct session *shmaddr)
{
    int n, i;
    srand(time(0));

    n = 0;
    while (n < MAXSESS - 1)
    {
        str[n] = rand() % 26 + 'A';
        n++;
    }
    str[n] = '\0';

    for (n = 0, i = 0; i < s_usernum; i++)
    {
        if (shmaddr[i].name[0] == 0)
            n = i;
        else if (strncmp(name, shmaddr[i].name, strlen(name)) == 0)
        {
            strncpy(str, shmaddr[i].sess, MAXSESS);
            return i;
        }
    }

    strncpy(shmaddr[n].sess, str, MAXSESS);
    strncpy(shmaddr[n].name, name, strlen(name));

    return n;
}

/********
 * 客户端响应函数
 * @param connfd accept返回的套接字描述符
 * @param shmid 共享内存ID
 * @parm info 配置文件信息(含用户名和密码)
*********/
void cli_echo(int connfd, int shmid, struct config_info *info)
{
    struct session *shmaddr;
    char buf[MAXBUFLEN];
    char sess[MAXSESS];
    int idx;
    int idxa;
    if ((shmaddr = (struct session *)shmat(shmid, 0, 0)) == NULL)
    {
        perror("shamt error");
        return;
    }

    Readline(connfd, buf);

    idx = find_sign('?', buf);
    //perror("cli_echo");
    //printf("%s",buf);
    if (strncmp(buf, "LOGIN", idx) == 0)
    {
        char name[MAXSTRLEN];
        char passwd[MAXSTRLEN];
        int idxa;

        idx = find_sign('=', buf + idx + 1) + idx + 2;
        idxa = find_sign('&', buf + idx + 1) + idx + 1;
        strncpy(name, buf + idx, idxa - idx);
        name[idxa - idx] = '\0';

        idx = find_sign('=', buf + idxa + 1) + idxa + 2;
        idxa = find_sign('\n', buf + idx + 1) + idx + 1;
        strncpy(passwd, buf + idx, idxa - idx);
        passwd[idxa - idx] = '\0';

        if (login_confirm(name, passwd, info) == 1)
        {
            //生成 储存 发送sess
            session_generate(sess, name, shmaddr);

            sprintf(buf, "sess=%s\n", sess);
            write(connfd, buf, strlen(buf));
        }
        else
        {
            //发送 login failed
            sprintf(buf, "login failed\n");
            write(connfd, buf, strlen(buf));
        }
    }
    else if (strncmp(buf, "TIME", idx) == 0)
    {
        //认证session
        //发送时间
        //perror("TIME\n");
        int i = 0;
        time_t ticks;

        idx = find_sign('=', buf) + 1;
        idxa = find_sign('\n', buf);
        strncpy(sess, buf + idx, idxa - idx);
        sess[idxa - idx] = 0;

        while (i < s_usernum)
        {
            if ((strncmp(sess, shmaddr[i].sess, strlen(sess))) == 0)
            {
                ticks = time(NULL);
                sprintf(buf, "%.24s\n", ctime(&ticks));
                //printf("%s\n", buf);
                write(connfd, buf, strlen(buf));
                close(connfd);
                return;
            }
            i++;
        }

        write(connfd, "Authentication failed\n", 22);
    }
    else if (strncmp(buf, "QUIT", idx) == 0)
    {
        //认证session
        //注销session
        int i = 0;

        idx = find_sign('=', buf) + 1;
        idxa = find_sign('\n', buf);
        strncpy(sess, buf + idx, idxa - idx);
        sess[idxa - idx] = 0;

        while (i < s_usernum)
        {
            if ((strncmp(sess, shmaddr[i].sess, strlen(sess))) == 0)
            {
                shmaddr[i].name[0] = 0;
                write(connfd, "Success\n", sizeof("Success\n") - 1);
                close(connfd);
                return;
            }
            i++;
        }

        write(connfd, "Authentication failed\n", sizeof("Authentication failed\n") - 1);
    }

    close(connfd);
    return;
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t child_pid;
    socklen_t clilen;
    struct config_info info;
    struct sockaddr_in servaddr, cliaddr;
    int shmid, shmkey;
    struct session *shmaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: $ %s <config path>\n", argv[0]);
        return -1;
    }

    init(argv[1], &info);

    shmkey = ftok(IPCPATH, IPCID);
    shmid = shmget(shmkey, (sizeof(struct session)) * s_usernum, IPC_CREAT | 0660);

    if ((shmaddr = (struct session *)shmat(shmid, 0, 0)) == NULL)
    {
        perror("shamt error");
        return -1;
    }

    memset(shmaddr, 0, sizeof(struct session) * s_usernum);
    shmdt(shmaddr);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return -1;
    }
    
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(info.port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind error");
        return -1;
    }

    if (listen(listenfd, 10) < 0)
    {
        perror("listen error");
        return -1;
    }

    signal(SIGCHLD, sig_chld);
    while (1)
    {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror("accept error");
            return -1;
        }
        if ((child_pid = fork()) < 0)
        {
            perror("fork error");
            return -1;
        }

        if (child_pid == 0)
        {
            close(listenfd);
            cli_echo(connfd, shmid, &info);
            exit(0);
        }

        close(connfd);
    }
    return 0;
}
