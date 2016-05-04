/*
 * use pipo to communicate between server and client
 * */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define _err(msg) \
{\
        fprintf(stderr, "error : %s -- %s\n", msg, strerror(errno)); \
        exit(-1); \
}

#define MAX_SIZE 1024
void client(int readfd, int writefd);
void server(int readfd, int writefd);
int
main(void)
{
        int fd1[2], fd2[2];
        pid_t pid;

        pipe(fd1);
        pipe(fd2);

        if((pid = fork()) < 0)
                _err("fork fail");
        if(pid > 0)
        {
                /*parent*/
                close(fd1[1]);
                close(fd2[0]);
                server(fd1[0], fd2[1]);
                printf("server stoped\n");
                waitpid(pid, NULL, 0);
        }
        else
        {
                close(fd1[0]);
                close(fd2[1]);
                client(fd2[0], fd1[1]);
                printf("client stoped\n");
                exit(0);
        }
        return 0;
}
void
client(int readfd, int writefd)
{
        char buffer[MAX_SIZE];
        size_t size;
        printf("client is start ...\n");
        printf("input the filename : ");
        fgets(buffer, MAX_SIZE, stdin);
        size = strlen(buffer);
        if(buffer[size - 1] == '\n')
                size--;
        if(write(writefd, buffer, size) <= 0)
                _err("write to server fail");
        while((size = read(readfd, buffer, MAX_SIZE)) > 0)
        {
                buffer[size] = '\0';
                printf("%s", buffer);
                fflush(stdout);
        }
        close(readfd);
        close(writefd);
}
void
server(int readfd, int writefd)
{
        char buffer[MAX_SIZE];
        int fd;
        size_t size;
        printf("server start ...\n");
        if((size = read(readfd, buffer, MAX_SIZE)) <= 0)
                _err("read pipe fail");
        buffer[size] = '\0';
        if(access(buffer, F_OK | R_OK) == -1)
        {
                strcpy(buffer, "file is not exists or can not be read");
                if(write(writefd, buffer, strlen(buffer)) <= 0)
                        _err("test file fail");
        }
        else
        {
                printf("file name = %s\n", buffer);
                if((fd = open(buffer, O_RDONLY)) == -1)
                        _err("open file fail");
                while((size = read(fd, buffer, MAX_SIZE)) > 0)
                {
                        if(write(writefd, buffer, size) <= 0)
                                _err("write to client fail");
                }
                close(fd);
        }
        close(writefd);
        close(readfd);
}