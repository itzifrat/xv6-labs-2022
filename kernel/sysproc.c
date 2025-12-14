#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

#include "fs.h"
#include "file.h"


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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


// setpriority(int pid, int newprio)
// Returns previous priority on success, or 1 on failure.

uint64
sys_setpriority(void)
{
  int pid, newprio;
  if (argint(0, &pid) < 0)
    return 1;
  if (argint(1, &newprio) < 0)
    return 1;

  if (newprio < 0 || newprio > 10)
    return 1;

  struct proc *p;
 
  for (p = proc; p < &proc[NPROC]; p++) {
  
    if (p->pid == pid) {
      int old = p->priority;
      p->priority = newprio;
      return old;
    }
  }
  return 1;
}

// setpname(const char *newname)
// Copies newname from user space and sets myproc()->name.
// Returns 0 on success, 1 on failure.
uint64
sys_setpname(void)
{
  char buf[16];
  if (argstr(0, buf, sizeof(buf)) < 0)
    return 1;

  struct proc *p = myproc();
  safestrcpy(p->name, buf, sizeof(p->name));
  return 0;
}

// filesize(const char *path)
// Returns:
//   >=0  : size in bytes (on success)
//   -1   : file does not exist
//   -2   : not a regular file
uint64
sys_filesize(void)
{
  char path[128];
  if (argstr(0, path, sizeof(path)) < 0)
    return (uint64)-1;

  struct inode *ip = namei(path);
  if (ip == 0)
    return (uint64)-1; // file does not exist

  ilock(ip);
  if (ip->type != T_FILE) {
    iunlockput(ip);
    return (uint64)-2; // not a regular file
  }
  int size = ip->size;
  iunlockput(ip);
  return (uint64)size;
}
