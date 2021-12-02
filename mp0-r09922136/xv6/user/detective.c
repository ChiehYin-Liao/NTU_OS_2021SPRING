#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int
detective(char *path, char *name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  int find = 0;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    exit(1);
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }

  if (st.type == T_DIR){

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      exit(1);
    }

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (!strcmp(de.name, ".") || !strcmp(de.name, "..") ){
        continue;
      }

      if (strcmp(de.name, name) == 0) {
	find += 1;
        printf("%d as Watson: %s\n", getpid(), buf);
      }
      
      find += detective(buf, name);
    }
  }
  close(fd);
  return find;
}

int
main(int argc, char *argv[])
{
  int pipefds[2];
  int returnstatus;
  int pid;
  
  if (argc != 2){
      exit(1);
  } 

   returnstatus = pipe(pipefds);
   if (returnstatus == -1) {
      printf("Unable to create pipe\n");
      exit(1);
   }

   pid = fork();

   //child process: Watson
   if (pid == 0) {
      if (detective(".", argv[1])) {
          write(pipefds[1], "y\n", 2);
      } else {
          write(pipefds[1], "n\n", 2);
      }
   } else { //parent process: Holmes
       char msg[1];
       read(pipefds[0], msg, 1);

       if (strcmp(msg, "y") == 0) {
          printf("%d as Holmes: This is the evidence\n", getpid());
       } 
       else {
          printf("%d as Holmes: This is the alibi\n", getpid());
       }

   }
  exit(0);
}
