#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<pwd.h>
#include<grp.h>
#include<string.h>
#include<dirent.h>
#include<time.h>
#include<linux/kdev_t.h>
#include"mylist.h"

#define K_SIZE 1000
#define M_SIZE 1000000

typedef struct filenode
{
	struct stat st;
	char filename[100];
}FileNode;

int show_all = 0;//显示所有的文件名(包括.和..)
int show_list = 0;//显示文件的信息
int show_recursion = 0;//是否递归
int show_A = 0;//显示所有的文件名(不包括.和..)
int reverse = 0;//是否反序
int color = 0;//控制颜色
int readable = 0;//文件大小是否可读
int i_node  = 0;//是否带节点号

const mode_t IF_MODE[] = {S_IFREG,S_IFDIR,S_IFLNK,S_IFBLK,S_IFCHR,S_IFIFO,S_IFSOCK};//文件的七种类型

int maxlengthofpw = 0;
int maxlengthofgr = 0;
int maxlengthofsize = 0;
int maxlengthoflink = 0;
int maxlengthofinode = 0;

int compare_filename(void *filea,void *fileb);
int (*cmp)(void *,void *) = compare_filename;

void show_file_info(FileNode *file);
void show_file_name(FileNode *file);

void dele(char *s)
{
	int k = 0,i = 0;
	while(s[i] != '\0')
	{
		if(((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') ||(s[i] >= '0' && s[i] <= '9')))
		{
			s[k] = s[i];
			k++;
		}
		i++;
	}
	s[k+1] = '\0';
}

int length_filesize(long size)
{
	int length = 0;
	while(size > 0)
	{
		size = size / 10;
		length++;
	}
	return length;
}
int length_link(int link)
{
	int length = 0;
	while(link > 0)
	{
		link = link / 10;
		length++;
	}
	return length;
}

//获取最长的文件大小宽度
int maxlength_filesize(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		long size = (long)(file->st.st_size);
		if(length_filesize(size) > length)
			length = length_filesize(size);
		p = p->next;
	}
	return length;
}
//获取最长的链接宽度
int maxlength_link(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		int link = (int)(file->st.st_size);
		if(length_link(link) > length)
			length = length_link(link);
		p = p->next;
	}
	return length;
}
//获取最长的节点号宽度
int maxlength_inode(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		int inode = (int)(file->st.st_ino);
		if(length_link(inode) > length)
			length = length_link(inode);
		p = p->next;
	}
	return length;
}
//获取最长的用户id拥有者的宽度
int maxlength_pw(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		struct passwd *pw;
		pw = getpwuid(file->st.st_uid);
		if(strlen(pw->pw_name) > length)
			length = strlen(pw->pw_name);
		p = p->next;
	}
	return length;
}
//获取最长的组id拥有者的宽度
int maxlength_gr(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		struct group *gr;
		gr = getgrgid(file->st.st_gid);
		if(strlen(gr->gr_name) > length)
			length = strlen(gr->gr_name);
		p = p->next;
	}
	return length;
}
int maxlength_major(List *h)
{
	Node *p = h->first;
	int length = 0;
	while(p != NULL)
	{
		FileNode *file = (FileNode *)p->data;
		if(S_ISCHR(file->st.st_mode) || S_ISBLK(file->st.st_mode))
		{
			if(length_link((int)MAJOR(file->st.st_rdev)) > length)
				length = length_link((int)MAJOR(file->st.st_rdev));
		}
		p = p->next;
	}
	return length;
}
//文件大小可视化
void get_size_readable(long size,char buff[])
{
	if(size < K_SIZE)
	{
		sprintf(buff,"%ld",size);
	}
	else if(size >= K_SIZE && size < K_SIZE * 10)
	{
		sprintf(buff,"%.1fK",size / (K_SIZE * 1.0));
	}
	else if(size >= K_SIZE * 10 && size < M_SIZE)
	{
		sprintf(buff,"%ldK",(size + K_SIZE / 2) / K_SIZE );
	}
	else if(size >= M_SIZE && size < M_SIZE * 10)
	{
		sprintf(buff,"%.1fM",size / (M_SIZE * 1.0));
	}
	else
	{
		sprintf(buff,"%ldM",(size + M_SIZE / 2) / M_SIZE);
	}
}
//按文件名排序
int compare_filename(void *filea,void *fileb)
{
	FileNode *Filea = (FileNode *)filea;
	FileNode *Fileb = (FileNode *)fileb;
	if((strcmp(Filea->filename,Fileb->filename)) < 0)
		return 1;
	return 0;
}
//判断是否为目录文件
int judge_dir(char *pathname)
{
	struct stat st;
	if((lstat(pathname,&st)) < 0)
	{
		perror("stat\n");
		printf("erro in judge_dir lstat!\n");
		exit(1);
	}
	if(S_ISDIR(st.st_mode))
	{
		return 1;
	}
	else
		return 0;
}
//按修改时间排序
int compare_file_change_time(void *filea,void *fileb)
{
    FileNode *Filea = (FileNode *)filea;
    FileNode *Fileb = (FileNode *)fileb;
	if(Filea->st.st_mtime > Fileb->st.st_mtime)
		return 1;
	else
		return 0;
}
//按文件大小排序
int compare_file_size(void *filea,void *fileb)
{
    FileNode *Filea = (FileNode *)filea;
    FileNode *Fileb = (FileNode *)fileb;
    if(Filea->st.st_size > Fileb->st.st_size)
        return 1;
    else
        return 0;
}
void show_allnohidefile_info(void *s)
{
	FileNode *file = (FileNode *)s;
	if((*file->filename) != '.')
	{
		show_file_info(file);
	}
}
void show_A_info(void *s)
{
	FileNode *file = (FileNode *)s;
	if(!((strcmp(file->filename,".")) == 0 || ((strcmp(file->filename,"..")) == 0)))
	{
		show_file_info(file);
	}
}
void show_allfile_info(void *s)
{
	FileNode *file = (FileNode *)s;
	show_file_info(file);
}
//根据文件类型添加颜色
int get_color(mode_t mode)
{
	const int color[] = {37,34,36,33,33,36,35};
	if(S_ISREG(mode) && (mode & S_IXUSR || mode & S_IXGRP || mode & S_IXOTH ))
	{
		return 32;
	}
	for(int i = 0;i < 7 ;i++)
	{
		if((mode & S_IFMT) == IF_MODE[i])
			return color[i];
	}
	return 37;
}
//显示文件名
void show_file_name(FileNode *file)
{
	struct stat temp_st;
	memset(&temp_st,0,sizeof(struct stat));
	if(color)
	{
		char filename_link[128] = {0};
		if(S_ISLNK(file->st.st_mode))
		{
			readlink(file->filename,filename_link,128);
			lstat(filename_link,&temp_st);
			printf("\033[36m%s\033[0m -> \033[%dm%s\033[0m",file->filename,get_color(file->st.st_mode),filename_link);
		}
		else
		{
			printf("\033[%dm%s \033[0m",get_color(file->st.st_mode),file->filename);	
		}
	}
	else
		printf("%s ",file->filename);
}

void show_allfile_name(void *s)
{
	FileNode *file = (FileNode *)s;
	show_file_name(file);

}

void show_allnohidefile_name(void *s)
{
	FileNode *file = (FileNode *)s;
	if((*file->filename) != '.')
	{
		show_file_name(file);
	}
}
void show_A_name(void *s)
{
	FileNode *file = (FileNode *)s;
	if(!((strcmp(file->filename,".")) == 0 || ((strcmp(file->filename,"..")) == 0)))
	{
		show_file_name(file);
	}
}
//显示文件信息
void show_file_info(FileNode *file)
{
	int flag = 0;
	char buff[10] = {0};
	struct passwd *pw;
	struct group *gr;
	if(i_node)
		printf("%*d ",maxlengthofinode,(int)file->st.st_ino);
	switch(file->st.st_mode & S_IFMT)
	{
		case S_IFREG:     //普通文件
			printf("-");
			break;
		case S_IFDIR:     //目录文件
			printf("d"); 
			break;
		case S_IFLNK:     
			printf("l");
			break;
		case S_IFBLK:
			printf("b");
			break;
		case S_IFCHR:
			printf("c");
			break;
		case S_IFIFO:
			printf("p");
			break;
		case S_IFSOCK:
			printf("s");
			break;
	}
	for(int i = 8 ;i >= 0; i--)
	{
		if(file->st.st_mode & (1 << i))
		{
			switch(i%3)
			{
				case 2:
					printf("r");
					break;
				case 1:
					printf("w");
					break;
				case 0:
					printf("x");
					break;
			}	
		}
		else
		{
			printf("-");
		}
	}

	pw = getpwuid(file->st.st_uid);
	gr = getgrgid(file->st.st_gid);

	printf(" %*d %-*s %-*s ",maxlengthoflink,(int)file->st.st_nlink,maxlengthofpw,pw->pw_name,maxlengthofgr,gr->gr_name);
	if(S_ISCHR(file->st.st_mode) || S_ISBLK(file->st.st_mode))
	{
		flag = 1;
		printf("%*d, %*d ",maxlengthofsize,(int)MAJOR(file->st.st_rdev),maxlengthofsize,(int)MINOR(file->st.st_rdev));
	}
	else
	{
		if(readable)
		{
			get_size_readable((long)file->st.st_size,buff);
			printf("%4s ",buff);
		}
		else
		{	if(flag)
				printf("%*ld ",2*maxlengthofsize,(long)file->st.st_size);
			else
				printf("%*ld ",maxlengthofsize,(long)file->st.st_size);
		}
	}
	printf("%.12s ",4 + ctime(&file->st.st_mtime));
	show_file_name(file);
	printf("\n");
}
//创建文件链表
void create_ls_list(List *h,char *pathname)
{		
	DIR *dir;
	struct dirent *dirent;
	struct stat sta;
	FileNode *file;
	if((dir = opendir(pathname)) == NULL)
	{
		printf("can't open dir in creat_ls_list :%s\n",pathname);
		exit(1);
	}
	chdir(pathname);
	int ret = 0;
	while((dirent = readdir(dir)) != NULL)
	{
		file = (FileNode *)malloc(sizeof(FileNode));
		ret = lstat(dirent->d_name,&sta);
		if(ret)
		{
			perror("stat");
			printf("erro in create_ls_list lstat\n");
			exit(1);
		}
		(file->st) = sta;
		strcpy(file->filename,dirent->d_name);
		ListInsertTail(h,file);
	}
	closedir(dir);
}

void do_ls(char *filename)
{
	List *h = CreateList();
    create_ls_list(h,filename);


	maxlengthofpw = maxlength_pw(h);
	maxlengthofgr = maxlength_gr(h);
    maxlengthofsize = maxlength_filesize(h);
	maxlengthofinode = maxlength_inode(h);

	ListSort(h,cmp);

	if(reverse)
		ListReverse(h);
	if(show_list)
	{
		if(show_all)
		{
			ListOutput(h,show_allfile_info);
		}
		else
		{ 
			if(show_A)
				ListOutput(h,show_A_info);
			else
				ListOutput(h,show_allnohidefile_info);
		}
	}
	else
	{
		if(show_all)
		{
			ListOutput(h,show_allfile_name);
			printf("\n");
		}
		else
		{
			if(show_A)
			{
				ListOutput(h,show_A_name);
				printf("\n");
			}
			else
			{
				ListOutput(h,show_allnohidefile_name);
				printf("\n");
			}
		}
	}

	maxlengthofpw = 0;
	maxlengthofgr = 0;
    maxlengthofsize = 0;
    color = 0;

	ListFree(h);
}

void do_ls_r(char *filename)
{
	DIR *dir;
	struct dirent *dirent;
	do_ls(filename);
	if((dir = opendir(filename)) == NULL)
	{
		printf("can't open dir in do_ls_r:%s\n",filename);
		exit(1);
	}
	chdir(filename);
	while((dirent = readdir(dir)) != NULL)
	{
		char fname[256];
		if(!((strcmp(dirent->d_name,".")) == 0 || ((strcmp(dirent->d_name,"..")) == 0)))
	//	if(dirent->d_name[0] != '.')
		{
			memset(fname,0,256);
			strcpy(fname,filename);
			strcat(fname,"/");
			strcat(fname,dirent->d_name);
			if(judge_dir(fname))
			{
				printf("%s:\n",fname);
				do_ls_r(fname);
			}
		}
	}
	chdir("..");
	closedir(dir);
}

int main(int argc ,char *argv[])
{
	int opt;

	while((opt = getopt(argc,argv,"aRrlAtchSi")) != -1)
	{
		switch(opt)
		{
			case 'a':
				show_all = 1;
				break;
			case 'R':
				show_recursion = 1;
				break;
			case 'l':
				show_list = 1;
				break;
			case 'A':
				show_A = 1;
				break;
			case 'r':
				reverse = 1;
				break;
			case 't':
				cmp = compare_file_change_time;
				break;
            case 'S':
                cmp = compare_file_size;
                break;
			case 'c':
				color = 1;
				break;
			case 'h':
				readable = 1;
				break;
			case 'i':
				i_node = 1;
				break;
			case '?':
				printf("fdfs\n");
				exit(1);
		}
	}
	if(show_recursion)
	{
		if(argc == optind)
		{
			do_ls_r(".");
		}
		else
		{
			for(;optind < argc ;optind++)
			{
				if(judge_dir(argv[optind]))
				{
					printf("%s:\n",argv[optind]);
					do_ls_r(argv[optind]);
				}
				else
				{
					printf("not a directory is a file\n");
					exit(1);
				}
			}
		}

	}
	else
	{
		if(argc == optind)
		{
			do_ls(".");
		}
		else
		{
			for(;optind < argc ;optind++)
			{
				if(judge_dir(argv[optind]))
				{
			    	printf("%s:\n",argv[optind]);
					do_ls(argv[optind]);
				}
				else
				{
					printf("not a directory id a file\n");
					exit(1);
				}
			}
		}
	}
	
}

