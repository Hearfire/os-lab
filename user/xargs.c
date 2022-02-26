#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc,char*argv[]){
    
    char* arg[64];//we pass the array to the command finally
    int argp;
    
    for(int i=1;i<argc;i++){
        arg[i-1]=argv[i];
    }//record the arguments followed
    argp=argc-1;


    char buf[64];int readsize;//the data in the standard input
    char curstring[128];int curstringp=0;
    char* curstringstart=curstring;
    while((readsize=read(0,buf,sizeof(buf)))>0){       

        for(int i=0;i<readsize;i++){
            if(buf[i]=='\n'){
                curstring[curstringp]='\0';//the end of string
                arg[argp++]=curstring;
                arg[argp]=0;//add a null as the end of array

                if(fork()==0){
                    exec(argv[1],arg);
                }
                wait(0);
                argp=argc-1;//the next run of command
                curstringp=0;   
                curstringstart=curstring;  
            }else if(buf[i]==' '){
                curstring[curstringp++]=0;
                arg[argp++]=curstringstart;
                curstringstart=curstring+curstringp;
            }else{
                curstring[curstringp++]=buf[i];
            }
        }


    }

    exit(0);
}