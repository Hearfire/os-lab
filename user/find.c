#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char* path){
	//static char buf[DIRSIZ+1];
	char*p;
	
	for(p=path+strlen(path);p>=path && *p!= '/';p--);
	p++;
	
	//blank-padded
	//if(strlen(p)>=DIRSIZ)
	return p;
	
	//memmove(buf,p,strlen(p));
	//memset(buf+strlen(p),' ',DIRSIZ-strlen(p));
	//return buf;

}

void find(char*path,char*name){//find file name in the directory dir
   
	char buf[512],*p;
	int fd;
	struct stat st;
	struct dirent de;

	
	if((fd=open(path,0))<0){
		fprintf(2,"find:cannot open %s\n",path);
   		return;
	}
	if(fstat(fd,&st)<0){
		fprintf(2,"find:cannot stat %s\n",path);
		close(fd);
		return;
	}

	
	switch(st.type){
	case T_FILE:		
		if(!strcmp(fmtname(path),name)){
			printf("%s\n",path);}
			break;
	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      			printf("ls: path too long\n");
      			break;
    		}
    		
    		strcpy(buf,path);
    		p=buf+strlen(buf);
    		*p++='/';
    		while(read(fd,&de,sizeof(de))==sizeof(de)){
    			if(de.inum==0)
    				continue;//no file in this directory
    			
    			if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
            			continue;
        		
    			
    			memmove(p,de.name,DIRSIZ);
    			p[DIRSIZ]=0;
    			
    			if(stat(buf,&st)<0){
    				printf("ls: cannot stat %s\n",buf);
    				continue;
			}
			find(buf,name);	
    		
    		}
    		break;
	}
	
	close(fd);
}


int main(int argc, char *argv[])
{
	
	if(argc!=3){
		printf("please provide dir and name\n");
		exit(0);
	}
 	find(argv[1] ,argv[2]);
	exit(0);
}

