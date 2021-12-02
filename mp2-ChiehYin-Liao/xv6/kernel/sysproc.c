#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "stat.h"

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
  //myproc()->sz += n;
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


uint64
sys_mmap(void)
{
  uint64 addr;
  int length, prot, flags, fd, offset;
  if(argaddr(0, &addr) < 0 || argint(1, &length) < 0 || argint(2, &prot) < 0 || argint(3, &flags) < 0 || argint(4, &fd) < 0 || argint(5, &offset) < 0){
    return -1;
  }

  // addr will always be zero in MP2
  if(addr != 0)
    panic("mmap: addr not 0");

  // offset will always be zero in MP2
  if(offset != 0)
    panic("mmap: offset not 0");

  struct proc *p = myproc();
  struct file* f = p->ofile[fd];
  int pte_flag = PTE_U;  

  if (prot & PROT_READ) {
    if(!f->readable) return -1; // unreadable file
    pte_flag |= PTE_R;
  }

  // should be able to map file opened read-only with private writable mapping
  if (prot & PROT_WRITE) {
    if(!f->writable && (flags & MAP_SHARED)) return -1; // unwritable file
    pte_flag |= PTE_W;
  }

  struct vma* vma = vma_alloc();
  vma->length = length;
  vma->page_prot = pte_flag;
  vma->flags = flags;
  vma->file = f;
  vma->off = offset;

  // add the VMA to the process’s table of mapped regions
  struct vma* p_vma = p->vma;
  if(p_vma == NULL){ // add first VMA to the process's table of mapped regions
    vma->start = VMA_START;
    vma->next = NULL;
    p->vma = vma;

  } else { // add VMA to the tail of the table of mapped regions
    while(p_vma->next != NULL) {
        p_vma = p_vma->next;
    }
    vma->start = PGROUNDUP(p_vma->end);
    p_vma->next = vma;
    vma->next = NULL;
  }
  
  vma->end = vma->start + length;

  // increase the file’s reference count so that the structure 
  // doesn’t disappear when the file is closed
  vma->file->ref++;
  addr = vma->start;

  release(&vma->lock);
  return addr;
}


uint64
sys_munmap(void)
{
  uint64 addr;
  int length;
  if(argaddr(0, &addr) < 0 || argint(1, &length) < 0){
    return -1;
  }

  struct proc *p = myproc();
  struct vma *vma = p->vma;
  struct vma *pre = NULL;

  // find the VMA to unmap
  while(vma != NULL){
    if(addr >= vma->start && addr < vma->end) {
      break;
    } 
    pre = vma;
    vma = vma->next;
  }
  if(vma == NULL) {
    return -ERROR_MMAP_NOT_A_ADDR; 
  }

  // only can munmap from the start or end that is not at the middle of a vma
  if(addr != vma->start && addr + length != vma->end) {
    panic("munmap: address is invalid, that is at the middle of a vma");
  }

  // only write back MAP_SHARED pages that the program actually modified
  if((vma->page_prot & PTE_W) && (vma->flags & MAP_SHARED)) {
    // Write back to file from vma->start
    filewrite(vma->file, vma->start, length);
  }

  // uvmunmap the pages
  int npages = length / PGSIZE;
  uvmunmap(p->pagetable, PGROUNDDOWN(addr), npages, 1);
 
  // maintain the process’s table of mapped regions
  // and the informations of VMAs
  acquire(&vma->lock);
  if(addr == vma->start) {
    // munmap from the start of a VMA
    if (length == vma->length) { 
      // by all vma is munmap
      // edit process’s table of mapped regions
      vma->file->ref--;
      if (pre != NULL) {
        pre->next = vma->next;
        vma->next = NULL;
      } else {
        p->vma = vma->next;//first VMA
      }
    } else {
      vma->start += length;
      vma->off += length;
    }
  } else {
    // munmap from the end of a VMA
    vma->end -= length;
  }
  vma->length -= length;
  release(&vma->lock);
  return 0;
}

