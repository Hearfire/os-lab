#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primesieve(int* p1){
   int n,nprime;
   int p2[2];
   
   
   int num=-1;//indicates the first data
   
   pipe(p2);
   while(read(p1[0],&n,4)){//when there is no data in pipe,read returns 0
        //if(n==0)break;
        if(num==-1){
            nprime=n;
            printf("prime %d\n",n);
            num=0;
        }else{
           if(n%nprime!=0){
              write(p2[1],&n,4);
              num++;
           }           
        }  
   }   
   close(p1[0]);
   close(p2[1]);
   
   
   if(num>0){//pipe have data,fork and solve
     if(fork()==0){
       primesieve(p2);
     }else{
     wait(0);
     exit(0);
     
     }
     
   }else{//no data,end recur
   close(p1[1]);
   close(p2[0]);
   exit(0);   
   }
}


int main(){
   int i;
   int p[2];   
   pipe(p);
   
   for(i=2;i<=35;i++){
      write(p[1],&i,4);
   }
   close(p[1]);
   
   primesieve(p);
   close(p[0]);
   
 
   exit(0);


}

