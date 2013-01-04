#include "local.h"
#include <unistd.h>
#include <pty.h>
#include <fcntl.h>

int pid;
int flag;
int fd;

int local_open(char *a,char *b,char *c) {
  pid = forkpty(&fd,0,0,0);
  flag = fcntl(fd,F_GETFL,0);

  char *termset = "TERM=xterm";
  putenv(termset);

  printf("fd: %d",fd);
  if(pid == 0) {
    char args[3];
    args[0] = "/bin/bash";
    args[1] =""; 
    args[2] = 0;

    execl("/bin/bash","bash",NULL);
    return 1;
  }

  printf("fd: %d\n",fd);
  return 1;
}

int local_write(char *buffer,int len) {
  return write(fd,buffer,len);
}

int local_read(char *buffer,int len) {
  return read(fd,buffer,len);
}

int local_resize(int rows,int cols) {
  struct winsize size = { rows, cols, 0, 0 };
  ioctl(fd, TIOCSWINSZ, &size);
}

int local_close() {
  close(fd);
}
