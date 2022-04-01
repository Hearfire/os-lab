#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;

  backtrace();
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 
sys_sigalarm(void){
  int ticks;
  uint64 handler;
  struct proc* p=myproc();
  
  if(argint(0,&ticks)<0)
    return -1;

  if(argaddr(1,&handler)<0)
    return -1;

  p->interval=ticks;
  p->handler=handler;
  

  return 0;
}
uint64 
sys_sigreturn(void){

  struct proc*p=myproc();
  p->flag=0;  

  //restore reg

  //save reg
       p->trapframe->kernel_satp=p->mytrapframe->kernel_satp; 
       p->trapframe->kernel_sp=p->mytrapframe->kernel_sp;
       p->trapframe->kernel_hartid=p->mytrapframe->kernel_hartid;
       p->trapframe->kernel_trap=p->mytrapframe->kernel_trap;
       p->trapframe->epc=p->mytrapframe->epc;
       p->trapframe->ra =p->mytrapframe->ra   ;
       p->trapframe->sp   =p->mytrapframe->sp   ;
       p->trapframe->gp   =p->mytrapframe->gp   ;
       p->trapframe->tp  =p->mytrapframe->tp   ;
       p->trapframe->t0   =p->mytrapframe->t0   ;
       p->trapframe->t1   =p->mytrapframe-> t1  ;
       p->trapframe->t2   =p->mytrapframe-> t2  ;
       p->trapframe-> s0  =p->mytrapframe-> s0  ;
       p->trapframe-> s1  =p->mytrapframe-> s1  ;
       p->trapframe-> a0  =p->mytrapframe->  a0 ;
       p->trapframe-> a1  =p->mytrapframe->  a1 ;
       p->trapframe->  a2 =p->mytrapframe-> a2  ;
       p->trapframe-> a3  =p->mytrapframe->  a3 ;
       p->trapframe-> a4  =p->mytrapframe-> a4  ;
       p->trapframe-> a5  =p->mytrapframe-> a5  ;
       p->trapframe->  a6 =p->mytrapframe->  a6 ;
       p->trapframe-> a7  =p->mytrapframe-> a7  ;
       p->trapframe->  s2 =p->mytrapframe-> s2  ;
       p->trapframe->  s3 =p->mytrapframe->  s3 ;
       p->trapframe->  s4 =p->mytrapframe-> s4  ;
       p->trapframe-> s5  =p->mytrapframe-> s5  ;
       p->trapframe->  s6 =p->mytrapframe-> s6  ;
       p->trapframe-> s7  =p->mytrapframe->  s7 ;
       p->trapframe->  s8 =p->mytrapframe->  s8 ;
       p->trapframe->  s9 =p->mytrapframe-> s9  ;
       p->trapframe->  s10 =p->mytrapframe-> s10  ;
       p->trapframe-> s11  =p->mytrapframe->  s11 ;
       p->trapframe-> t3  =p->mytrapframe-> t3  ;
       p->trapframe->  t4 =p->mytrapframe->  t4 ;
       p->trapframe->  t5 =p->mytrapframe-> t5  ;
       p->trapframe-> t6  =p->mytrapframe-> t6  ;

  return 0;
}