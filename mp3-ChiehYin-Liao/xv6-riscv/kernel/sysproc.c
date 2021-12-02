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


// for mp3
uint64
sys_thrdstop(void)
{
  int interval, thrdstop_context_id;
  uint64 handler;
  if (argint(0, &interval) < 0)
    return -1;
  if (argint(1, &thrdstop_context_id) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;

  struct proc *p = myproc();

  // printf("[thrdstop start] interval:%d, thrdstop_context_id:%d\n", interval, thrdstop_context_id);

  p->thrdstop_ticks = 0;
  p->thrdstop_interval = interval;
  p->thrdstop_handler_pointer = handler;
  
  // save context(thrdstop_context_id) then jump to handler
  if (thrdstop_context_id != -1) {
    p->thrdstop_context_id = thrdstop_context_id;
    return thrdstop_context_id;
  } else {
    for (int i = 0; i < MAX_THRD_NUM; i++) {
      if (p->thrdstop_context_used[i] == 0) {
        p->thrdstop_context_used[i] = 1;
        p->thrdstop_context_id = i;
        return i;
      }
    }
    // if reaches here then thrdstop_context_used is full!
    return -1;
  }
}

// for mp3
uint64
sys_cancelthrdstop(void)
{

  int thrdstop_context_id;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;
  // printf("[cancelthrdstop start] thrdstop_context_id:%d\n", thrdstop_context_id);

  struct proc *p = myproc();

  if (thrdstop_context_id > -1 && thrdstop_context_id < MAX_THRD_NUM) {
      p->thrdstop_context[thrdstop_context_id].epc = p->trapframe->epc;
      p->thrdstop_context[thrdstop_context_id].ra = p->trapframe->ra;
      p->thrdstop_context[thrdstop_context_id].sp = p->trapframe->sp;
      p->thrdstop_context[thrdstop_context_id].gp = p->trapframe->gp;
      p->thrdstop_context[thrdstop_context_id].tp = p->trapframe->tp;
      p->thrdstop_context[thrdstop_context_id].s_regs[0] = p->trapframe->s0;
      p->thrdstop_context[thrdstop_context_id].s_regs[1] = p->trapframe->s1;
      p->thrdstop_context[thrdstop_context_id].s_regs[2] = p->trapframe->s2;
      p->thrdstop_context[thrdstop_context_id].s_regs[3] = p->trapframe->s3;
      p->thrdstop_context[thrdstop_context_id].s_regs[4] = p->trapframe->s4;
      p->thrdstop_context[thrdstop_context_id].s_regs[5] = p->trapframe->s5;
      p->thrdstop_context[thrdstop_context_id].s_regs[6] = p->trapframe->s6;
      p->thrdstop_context[thrdstop_context_id].s_regs[7] = p->trapframe->s7;
      p->thrdstop_context[thrdstop_context_id].s_regs[8] = p->trapframe->s8;
      p->thrdstop_context[thrdstop_context_id].s_regs[9] = p->trapframe->s9;
      p->thrdstop_context[thrdstop_context_id].s_regs[10] = p->trapframe->s10;
      p->thrdstop_context[thrdstop_context_id].s_regs[11] = p->trapframe->s11;
      p->thrdstop_context[thrdstop_context_id].t_regs[0] = p->trapframe->t0;
      p->thrdstop_context[thrdstop_context_id].t_regs[1] = p->trapframe->t1;
      p->thrdstop_context[thrdstop_context_id].t_regs[2] = p->trapframe->t2;
      p->thrdstop_context[thrdstop_context_id].t_regs[3] = p->trapframe->t3;
      p->thrdstop_context[thrdstop_context_id].t_regs[4] = p->trapframe->t4;
      p->thrdstop_context[thrdstop_context_id].t_regs[5] = p->trapframe->t5;
      p->thrdstop_context[thrdstop_context_id].t_regs[6] = p->trapframe->t6;
      p->thrdstop_context[thrdstop_context_id].a_regs[0] = p->trapframe->a0;
      p->thrdstop_context[thrdstop_context_id].a_regs[1] = p->trapframe->a1;
      p->thrdstop_context[thrdstop_context_id].a_regs[2] = p->trapframe->a2;
      p->thrdstop_context[thrdstop_context_id].a_regs[3] = p->trapframe->a3;
      p->thrdstop_context[thrdstop_context_id].a_regs[4] = p->trapframe->a4;
      p->thrdstop_context[thrdstop_context_id].a_regs[5] = p->trapframe->a5;
      p->thrdstop_context[thrdstop_context_id].a_regs[6] = p->trapframe->a6;
      p->thrdstop_context[thrdstop_context_id].a_regs[7] = p->trapframe->a7;
      
  }
  // p->thrdstop_interval = -1;
  return p->thrdstop_ticks;
}

// for mp3
uint64
sys_thrdresume(void)
{
  
  int  thrdstop_context_id, is_exit;
  if (argint(0, &thrdstop_context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;
  // printf("[thrdresume start] thrdstop_context_id:%d is_exit:%d\n", thrdstop_context_id, is_exit);

  struct proc *p = myproc();
  // acquire(&myproc()->lock);
  if (is_exit == 0) {
    // load context from proc->thrdstop_context
    p->trapframe->epc = p->thrdstop_context[thrdstop_context_id].epc;
    p->trapframe->ra = p->thrdstop_context[thrdstop_context_id].ra;
    p->trapframe->sp = p->thrdstop_context[thrdstop_context_id].sp;
    p->trapframe->gp = p->thrdstop_context[thrdstop_context_id].gp;
    p->trapframe->tp = p->thrdstop_context[thrdstop_context_id].tp;
    p->trapframe->s0 = p->thrdstop_context[thrdstop_context_id].s_regs[0];
    p->trapframe->s1 = p->thrdstop_context[thrdstop_context_id].s_regs[1];
    p->trapframe->s2 = p->thrdstop_context[thrdstop_context_id].s_regs[2];
    p->trapframe->s3 = p->thrdstop_context[thrdstop_context_id].s_regs[3];
    p->trapframe->s4 = p->thrdstop_context[thrdstop_context_id].s_regs[4];
    p->trapframe->s5 = p->thrdstop_context[thrdstop_context_id].s_regs[5];
    p->trapframe->s6 = p->thrdstop_context[thrdstop_context_id].s_regs[6];
    p->trapframe->s7 = p->thrdstop_context[thrdstop_context_id].s_regs[7];
    p->trapframe->s8 = p->thrdstop_context[thrdstop_context_id].s_regs[8];
    p->trapframe->s9 = p->thrdstop_context[thrdstop_context_id].s_regs[9];
    p->trapframe->s10 = p->thrdstop_context[thrdstop_context_id].s_regs[10];
    p->trapframe->s11 = p->thrdstop_context[thrdstop_context_id].s_regs[11];
    p->trapframe->t0 = p->thrdstop_context[thrdstop_context_id].t_regs[0];
    p->trapframe->t1 = p->thrdstop_context[thrdstop_context_id].t_regs[1];
    p->trapframe->t2 = p->thrdstop_context[thrdstop_context_id].t_regs[2];
    p->trapframe->t3 = p->thrdstop_context[thrdstop_context_id].t_regs[3];
    p->trapframe->t4 = p->thrdstop_context[thrdstop_context_id].t_regs[4];
    p->trapframe->t5 = p->thrdstop_context[thrdstop_context_id].t_regs[5];
    p->trapframe->t6 = p->thrdstop_context[thrdstop_context_id].t_regs[6];
    p->trapframe->a0 = p->thrdstop_context[thrdstop_context_id].a_regs[0];
    p->trapframe->a1 = p->thrdstop_context[thrdstop_context_id].a_regs[1];
    p->trapframe->a2 = p->thrdstop_context[thrdstop_context_id].a_regs[2];
    p->trapframe->a3 = p->thrdstop_context[thrdstop_context_id].a_regs[3];
    p->trapframe->a4 = p->thrdstop_context[thrdstop_context_id].a_regs[4];
    p->trapframe->a5 = p->thrdstop_context[thrdstop_context_id].a_regs[5];
    p->trapframe->a6 = p->thrdstop_context[thrdstop_context_id].a_regs[6];
    p->trapframe->a7 = p->thrdstop_context[thrdstop_context_id].a_regs[7];

  } else {
    // thread exit
    p->thrdstop_context_used[thrdstop_context_id] = 0;
    p->thrdstop_interval = -1;
    
  }
  // release(&myproc()->lock);
  return 0;
}
