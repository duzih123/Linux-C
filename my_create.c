#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifdef File
int main()
{
    int fd;
    if((fd=open("example.c",O_CREAT|O_EXCL,S_IRUSR|S_IWUSR))==-1){
        //if((fd=creat("example.c",S_IRWXU))==-1){
        perror("open");
        exit(1);
    }else {
        printf("create file success\n");
    }
    close(fd);
    return 0;
}
#endif
