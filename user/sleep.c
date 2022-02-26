
//sleep.c: pause for a user-specified number of ticks

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
  int n=atoi(argv[1]);
  if(n==0){
    write(1,"sleep failed1\n",14);   
  }
  else{
    int x=sleep(n);
    if(x==-1){
      write(1,"sleep failed2\n",14);    
    }
    
  }


  exit(0);
}
