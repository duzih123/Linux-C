#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>

#define PARAM_NONE 0 //无参数
#define PARAM_A 1    //-a 显示所有文件
#define PARAM_L 2    // -l 一行只显示一个文件的详细信息
#define MAXROWLEN 80  //一项显示的最大字符数
int g_leave_len =MAXROWLEN;
int g_maxlen;

void my_err(const char * err_string,int line)
{
    fprintf(stderr,"line:%d",line);
    perror(err_string);
    exit(1);
}
void display_attribute(struct stat buf,char *name)
{
    char  buf_time[32];
    struct passwd *psd;
    struct group *grp;
    // 获取并打印文件类型
    if(S_ISLNK(buf.st_mode)){
        printf("1");
    }else if (S_ISREG(buf.st_mode)) {
        printf("-");
    }else if (S_ISDIR(buf.st_mode)) {
        printf("d");
    }else if (S_ISCHR(buf.st_mode)) {
        printf("c");
    }else if (S_ISBLK(buf.st_mode)) {
        printf("b");
    }else if (S_ISFIFO(buf.st_mode)) {
        printf("f");
    }else if (S_ISSOCK(buf.st_mode)) {
        printf("s");
    }

    //获取并打印文件所有者的权限
    if(buf.st_mode&S_IRUSR){
        printf("r");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IWUSR){
        printf("w");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IXUSR){
        printf("x");
    }else {
        printf("-");
    }

    //获取并打印与文件所有者同组的用户对文件的操作权限
    if(buf.st_mode&S_IRGRP){
        printf("r");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IWGRP){
        printf("w");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IXGRP){
        printf("x");
    }else {
        printf("-");
    }

    //获取并打印其他用户对该文件的操作权限
    if(buf.st_mode&S_IROTH){
        printf("r");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IWOTH){
        printf("w");
    }else {
        printf("-");
    }
    if(buf.st_mode&S_IXOTH){
        printf("x");
    }else {
        printf("-");
    }

    printf("   ");

    //根据uid与gid获取文件所有者的用户名与组名

    psd =getpwuid(buf.st_uid);
    grp =getgrgid(buf.st_gid);

    printf("%4d ",buf.st_nlink);
    printf("%-8s",psd->pw_name);
    printf("%-8s",grp->gr_name);

    printf("%6d",buf.st_size);
    strcpy(buf_time,ctime(&buf.st_mtime));
    buf_time[strlen(buf_time)-1]='\0';
    printf("  %s",buf_time);
}
/* 在没有使用-l 选项是，打印一个文件名 打印时上下行对齐*/
void dispaly_single(char *name)
{
    int i,len;
    if(g_leave_len<g_maxlen){
        printf("\n");
        g_leave_len =MAXROWLEN;
    }
    len =strlen(name);
    len =g_maxlen -len;
    printf("%-s",name);
    for(i=0;i<len;i++){
        printf(" ");
    }
    printf(" ");
    g_leave_len -=(g_maxlen+2);
}
void display(int flag,char *pathname)
{
    int i,j;
    struct stat buf;
    char name[NAME_MAX+1];
    for(i=0,j=0;i<strlen(pathname);i++){
        if(pathname[i]=='/'){
            j=0;
            continue;
        }
        name[j++] =pathname[i];
    }
    name[j]='\0';

    if(lstat(pathname,&buf)==-1){
        my_err("stat",__LINE__);
    }
    switch(flag){
        case PARAM_NONE:
        if(name[0]!='.'){
            dispaly_single(name);
        }
        break;
    case PARAM_A:
        dispaly_single(name);
        break;
    case PARAM_L:
        if(name[0]!='.'){
            display_attribute(buf,name);
            printf("   %-s\n",name);
        }
        break;
    case PARAM_A+PARAM_L:
        display_attribute(buf,name);
        printf("    %-s\n",name);
        break;
    default:
        break;
    }
}

void display_dir(int flag_param,char *path)
{
    DIR  *dir;
    struct dirent *ptr;
    int count =0;
    char filename[256][PATH_MAX+1],temp[PATH_MAX+1];
    dir =opendir(path);
    if(dir==NULL){
        my_err("opendir",__LINE__);
    }
    while ((ptr=readdir(dir))!=NULL) {
        if(g_maxlen<strlen(ptr->d_name))
            g_maxlen =strlen(ptr->d_name);
        count++;
    }
    closedir(dir);
    if(count>256)
        my_err("too many files under this dir",__LINE__);
    int i,j,len =strlen(path);

    dir =opendir(path);
    for(i=0;i<count;i++){
        ptr =readdir(dir);
        if(ptr==NULL){
            my_err("readdir",__LINE__);
        }
        strncpy(filename[i],path,len);
        filename[i][len]='\0';
        strcat(filename[i],ptr->d_name);
        filename[i][len+strlen(ptr->d_name)]='\0';
    }

    for(i=0;i<count-1;i++){
        for(j=0;j<count-1-i;j++){
            if(strcmp(filename[j],filename[j+1])>0){
                strcpy(temp,filename[j+1]);
                temp[strlen(filename[j+1])]='\0';
                strcpy(filename[j+1],filename[j]);
                filename[j+1][strlen(filename[j])]='\0';
                strcpy(filename[j],temp);
                filename[j][strlen(temp)]='\0';
            }
        }
    }
    for(i=0;i<count;i++)
        display(flag_param,filename[i]);
    closedir(dir);
    if((flag_param&PARAM_L)==0)
        printf("\n");
}

int main(int argc,char **argv)
{
    int i,j,k,num;
    char  path[PATH_MAX+1];
    char param[32];
    int flag_param =PARAM_NONE;
    struct stat buf;
    j=0;
    num=0;
    for(i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            for(k=1;k<strlen(argv[i]);k++,j++){
                param[j] =argv[i][k];
            }
            num++;
        }
    }

    for(i=0;i<j;i++){
        if(param[i]=='a'){
            printf("%d",__LINE__);
            flag_param |=PARAM_A;
            continue;
        }else if(param[i]=='l'){
            flag_param |=PARAM_L;
            continue;
        }else {
            printf("my_ls:invalid option -%c\n",param[i]);
            exit(1);
        }
    }
    param[j]='\0';

    if((num+1)==argc){
        strcpy(path,"./");
        path[2]='\0';
        display_dir(flag_param,path);
        return 0;
    }
    i=1;
    do{
        if(argv[i][0]=='-'){
            i++;
            continue;
        }else {
            strcpy(path,argv[i]);
            if(stat(path,&buf)==-1)
                my_err("stat",__LINE__);
            if(S_ISDIR(buf.st_mode)){
                if(path[strlen(argv[i])-1]!='/'){
                    path[strlen(argv[i])]='/';
                    path[strlen(argv[i])+1]='\0';
                }else {
                    path[strlen(argv[i])]='\0';
                }
                display_dir(flag_param,path);
                i++;
            }else {
                display(flag_param,path);
                i++;
            }
        }
    }while(i<argc);
    return 0;
}
